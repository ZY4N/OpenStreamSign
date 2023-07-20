#include <notifier.hpp>

#include <util/base64.hpp>
#include <util/for_each.hpp>
#include <platform/log.hpp>
#include <hmac_sha_512_engine.hpp>
#include <hmac_sha_512_handshake.hpp>
#include <aes_transceiver.hpp>

#include <thread>
#include <chrono>
#include <iomanip>
#include <iostream>

inline void logger_error_code(const char *origin, const std::error_code &e) {
	logger_error("[%s][%s]: %s", origin, e.category().name(), e.message().c_str());
}

notifier::notifier(const notifier_config_t& cfg) : config{ cfg }{};

std::error_code notifier::connect() {

	std::error_code e;

	const auto &connConfig = config.get<"connection">();
	const auto secretBase64 = connConfig.get<"secret">();

	constexpr auto base64Length = ztu::base64::encodedSize(64 + 32);
	if (secretBase64.length() != base64Length) {
		std::cout << "Wrong secret length" << std::endl;
		return { 1, std::system_category() };
	}


	std::cout << "decoding" << std::endl;
	ztu::base64::decode(secretBase64, secret);

	std::cout << "validator init" << std::endl;

	const auto sha512_secret = std::span{ secret.begin(), 512 / 8 };
	const auto aes256_secret = std::span{ sha512_secret.end(), 256 / 8 };

	if ((e = sha_engine.init(sha512_secret)))
		return e;

	if ((e = aes_engine.init(aes256_secret)))
		return e;

	std::cout << "starting communication thread" << std::endl;

	connectingThread = std::thread([this]() {
		using namespace std::chrono_literals;
		using clock = std::chrono::high_resolution_clock;
		using millis = std::chrono::milliseconds;

		const auto &connConfig = config.get<"connection">();
		const auto sleepIntervals = connConfig.get<"timeout_interval_ms">();

		auto connectionInterval = sleepIntervals.begin();
		auto connectionStart = clock::now();

		const auto &host = connConfig.get<"ip">();
		const auto &port = connConfig.get<"port">();
		logger_info("connecting... to %s %llu", host.c_str(), port);

		do {
			std::error_code e;

			if ((e = connection.connect(host, port))) {

				std::cout << "gave up" << std::endl;
				const auto timeSinceStart = std::chrono::duration_cast<millis>(
					clock::now() - connectionStart
				).count();

				if (
					timeSinceStart > static_cast<i64>(connectionInterval->get<"dt">()) and
					connectionInterval + 1 < sleepIntervals.end()
				) {
					++connectionInterval;
					logger_info("Changed retry interval to: %llu ms", connectionInterval->get<"interval">());
				}

				const auto sleepTill = (
					std::chrono::system_clock::now() +
					std::chrono::milliseconds(
						connectionInterval->get<"dt">()
					)
				);

				std::unique_lock<std::mutex> lock(intervalMutex);
				const auto timeout = intervalDisruptor.wait_until(lock, sleepTill) == std::cv_status::timeout;
				const auto stopThread = not timeout && not reconnect;
				lock.unlock();

				if (stopThread) {
					return; // killing connection thread
				}
				continue;
			}
			logger_error("connected");

			if ((e = this->handleHandshake())) {
				logger_error_code("HANDSHAKE_ERROR", e);
				continue;
			}

			this->onConnect();

			this->sendDecoyCommands();

			logger_error("Connection lost");

			connectionInterval = sleepIntervals.begin();
			connectionStart = clock::now();

		} while (reconnect);
	});

	return { 0, std::system_category() };
}

std::error_code notifier::handleHandshake() {
	std::lock_guard<std::mutex> guard(connectionMutex);
	return validator.validate(false);
}

void notifier::onConnect() {
	connected = true;

	const auto &animations = config.get<"animations">();

	using namespace string_literals;
	using enum sign_state;

	ztu::for_each::value<
		std::pair{ CONNECTED, "CONNECTED"_sl },
		std::pair{ RECORDING, "RECORDING"_sl },
		std::pair{ RECORDING_PAUSED, "RECORDING_PAUSED"_sl },
		std::pair{ STREAMING, "STREAMING"_sl },
		std::pair{ STREAMING_PAUSED, "STREAMING_PAUSED"_sl },
		std::pair{ IDLE, "IDLE"_sl },
		std::pair{ PROCESSING, "PROCESSING"_sl },
		std::pair{ SETUP, "SETUP"_sl }
	>([&]<auto state_name>() {
		sendMessage<sign_message_type::SET_ANIMATION>(
			state_name.first,
			animations.template get<state_name.second>()
		);
		return false;
	});

	sendMessage<sign_message_type::CHANGE_STATE>(state);
}

void notifier::sendDecoyCommands() {

	constexpr auto maxIntervalMs = 10 * 1000; // balance between good distribution and too much traffic
	constexpr auto deviationFixFactor = 0.3; // spring back after none uniform interval

	static long deviationMs = 0;

	while (connected) {

		static std::random_device rd;
		static std::mt19937 rng(rd());
		static std::uniform_int_distribution<long> dist(0, maxIntervalMs);

		auto intervalMs = dist(rng);

		if (deviationMs > 0) {
			// try to converge to uniform distribution without generating suspicious intervals
			std::uniform_int_distribution<long> correction(
				0,
				std::min(
					long(std::round(deviationFixFactor * double(deviationMs))),
					maxIntervalMs - intervalMs
				)
			);
			const auto correctionMillis =  correction(rng);
			intervalMs += correctionMillis;
			deviationMs -= correctionMillis;
		}

		const auto interval = std::chrono::milliseconds(intervalMs);

		const auto start = std::chrono::system_clock::now();
		const auto target = start + interval;

		std::unique_lock<std::mutex> lock(intervalMutex);
		const auto timeout = intervalDisruptor.wait_until(lock, target) == std::cv_status::timeout;
		lock.unlock();

		if (timeout) {
			sendMessage<sign_message_type::HEARTBEAT>();
		} else {
			// A real command got sent which changes the uniform distribution of packages.
			// Calculate deviation to converge against normal distribution with following packages.
			const auto end = std::chrono::system_clock::now();
			const auto actualInterval = end - start;
			const auto deltaT = interval - actualInterval;
			deviationMs += std::chrono::duration_cast<std::chrono::milliseconds>(deltaT).count();
		}
	}
}

void notifier::disconnect() {
	logger_error("disconnecting");

	// locking a mutex is not necessary
	reconnect = false;
	connected = false;

	// Interrupt possible sleep interval of the communication thread
	// to find out that it is should stop
	intervalDisruptor.notify_one();

	connectingThread.join();
	connection.disconnect();
}

notifier::~notifier() {
	disconnect();
}

void notifier::changeState(const sign_state &newState) {
	if (newState != state) {
		sendMessage<sign_message_type::CHANGE_STATE>(newState);
	}
	state = newState;
}


template<sign_message_type Type, typename... Args>
void notifier::sendMessage(Args&&... args) {
	std::unique_lock<std::mutex> lock(connectionMutex);
	if (connected) {
		const auto error = transceiver.send<Type>(std::forward<Args>(args)...);
		if (error) {
			logger_error_code("SENDING_ERROR", error);
			// Crude check to see if connection is lost
			// TODO find a safer solution with a platform independent interface
			if (error.category() == asio::system_category()) {
				connection.disconnect();
				connected = false;
			}
		} else {
			logger_info("Successfully sent packet of type %d", static_cast<int>(Type));
		}
	}
	lock.unlock();
	intervalDisruptor.notify_one(); // reset heartbeat interval
}
