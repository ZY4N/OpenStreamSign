#include <app.hpp>

#include <util/base64.hpp>
#include <util/for_each.hpp>
#include <platform/log.hpp>

#include <chrono>
#include <filesystem>


#include <iostream>

std::error_code app::start() {

	loadConfig("config.json");

	if (const auto error = initEncryptionEngines(); error)
		return error;

	connectionThread = std::thread([this] { handleConnection(); } );

	return { 0, std::system_category() };
}

void app::loadConfig(const std::string_view &configFileName) {
	auto path = obs_module_config_path(configFileName.data());
	if (not path) {
		config = default_app_config();
		return;
	}

	namespace fs = std::filesystem;

	const auto configFilename = fs::path(path);

	auto configDir = configFilename;
	configDir.remove_filename();

	if (not fs::exists(configDir))
		fs::create_directory(configDir);

	auto configInitialized = false;
	if (fs::exists(configFilename)) {
		try {
			auto is = std::ifstream(configFilename);
			auto parser = json::safe::parser(is);
			config = parser.parse<default_app_config>();
			configInitialized = true;
		} catch (const std::exception &e) {
			logger_warn("Error while parsing '%s': %s.\n Proceeding with default config", configFilename.c_str(), e.what());
			config = default_app_config();
		}
	}

	if (not configInitialized) {
		logger_info("Writing default config to '%s'\nPlease set the secret manually!", path);
		config = default_app_config();
		try {
			auto os = std::ofstream(configFilename);
			auto serializer = json::safe::serializer(os);
			serializer.serialize<default_app_config>(config);
			os.flush();
			os.close();
		} catch (const std::exception &e) {
			logger_warn("Error while writing default config to '%s': %s.", configFilename.c_str(), e.what());
			config = default_app_config();
		}
	}

	bfree(path);
}

std::error_code app::initEncryptionEngines() {

	const auto &connConfig = config.get<"connection">();
	const auto &secretBase64 = connConfig.get<"secret">();

	constexpr auto base64Length = ztu::base64::encodedSize(64 + 32);
	if (secretBase64.length() != base64Length) {
		logger_error("Wrong secret length: expected '%zu' got '%zu'", base64Length, secretBase64.length());
		return { 1, std::system_category() };
	}

	ztu::base64::decode(secretBase64, secret);

	const auto sha512_secret = std::span{ secret.begin(), 512 / 8 };
	const auto aes256_secret = std::span{ sha512_secret.end(), 256 / 8 };

	if (const auto error = sha_engine.init(sha512_secret); error)
		return error;

	if (const auto error = aes_engine.init(aes256_secret); error)
		return error;

	transceiver.engine() = &aes_engine;

	return { 0, std::system_category() };
}

void app::handleConnection() {
	using namespace std::chrono_literals;
	namespace chrono = std::chrono;
	using clock = chrono::high_resolution_clock;
	using millis = chrono::milliseconds;

	{
		const auto &animations = config.get<"animations">();
		const auto &connected = animations.get<"RECORDING">();

		std::array<u8, sizeof(sign_animation)> buffer;

		auto it = buffer.begin();
		sign_animation_transcoding::serialize(connected, it);
		const auto usedBytes = std::span{ buffer.begin(), it };

		logger_info("used %ld bytes", usedBytes.size());

		for (const auto byte : usedBytes) {
			std::cout << int(byte) << " ";
		}
		std::cout << std::endl;
	}


	const auto &connConfig = config.get<"connection">();
	const auto &sleepIntervals = connConfig.get<"timeout_interval_ms">();

	const auto &host = connConfig.get<"ip">();
	const auto &port = connConfig.get<"port">();

	logger_info("connecting... to %s %llu", host.c_str(), port);

	do {
		std::error_code error;

		auto lastConnected = clock::now();
		auto connTimeoutInterval = sleepIntervals.begin();

		while ((error = connection.connect(host, port))) {

			const auto timeDisconnected = chrono::duration_cast<millis>(
				clock::now() - lastConnected
			);

			const auto maxTimeDisconnected = millis(connTimeoutInterval->get<"dt">());
			const auto timeLeftInInterval = maxTimeDisconnected - timeDisconnected;

			if (
				timeDisconnected > maxTimeDisconnected and
				connTimeoutInterval + 1 < sleepIntervals.end()
			) {
				++connTimeoutInterval;
				logger_info("Changed retry interval to: %llu ms", connTimeoutInterval->get<"interval">());
			}

			//logger_info("waiting for: %llu ms", connTimeoutInterval->get<"interval">());

			const auto sleepTill = clock::now() + millis(connTimeoutInterval->get<"interval">());

			std::unique_lock<std::mutex> lock(intervalMutex);
			intervalDisruptor.wait_until(lock, sleepTill);
			const auto stopThread = not reconnect;
			lock.unlock();

			// killing connection thread
			if (stopThread) return;
		}

		logger_info("connected");

		if ((error = validateConnection())) {
			logger_error_code("HANDSHAKE_ERROR", error);
			continue;
		}

		if ((error = connection.setReceiveTimeoutInterval(3))) {
			logger_error_code("SOCKET_SETTING_TIMEOUT_ERROR", error);
			continue;
		}

		logger_info("connected");

		onConnect();

		sendDecoyCommands();

		logger_info("connection lost");

	} while (reconnect);
}

std::error_code app::validateConnection() {
	std::lock_guard<std::mutex> guard(connectionMutex);
	return hmac_sha_512_handshake::validate(sha_engine, connection, false);
}

void app::onConnect() {
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

void app::sendDecoyCommands() {

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

		logger_info("waiting heartbeat for interval");
		std::unique_lock<std::mutex> lock(intervalMutex);
		const auto timeout = intervalDisruptor.wait_until(lock, target) == std::cv_status::timeout;
		lock.unlock();
		logger_info("done waiting for interval");

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

void app::disconnect() {
	logger_error("disconnecting");

	// locking a mutex is not necessary
	reconnect = false;
	connected = false;

	// Interrupt possible sleep interval of the communication thread
	// to find out that it is should stop
	intervalDisruptor.notify_one();

	connectionThread.join();
	connection.disconnect();
}

app::~app() {
	disconnect();
}

void app::changeState(const sign_state &newState) {
	if (newState != state) {
		sendMessage<sign_message_type::CHANGE_STATE>(newState);
	}
	state = newState;
}


template<sign_message_type Type, typename... Args>
void app::sendMessage(Args&&... args) {
	std::unique_lock<std::mutex> lock(connectionMutex);
	if (connected) {
		std::error_code error;
		std::span<u8> message_packet;
		if ((error = transceiver.encrypt_message<Type>(
			message_packet, std::forward<Args>(args)...
		))) {
			logger_error_code("AES_PACKER", error);
		} else if ((error = connection.send(message_packet))) {
			logger_error_code("SENDING_ERROR", error);
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
