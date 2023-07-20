/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <fstream>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include "platform/log.hpp"
 #include "notifier.hpp"

#include <domain_logic/sign_transceiver.hpp>

#include <filesystem>
namespace fs = std::filesystem;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

notifier *notify;

static void onEvent(enum obs_frontend_event event, void *){
	switch (event) {
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
	case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED:
		logger_info("STARTED RECORDING");
		notify->changeState(sign_state::RECORDING);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_PAUSED:
		logger_info("RECORDING PAUSED");
		notify->changeState(sign_state::RECORDING_PAUSED);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
		logger_info("IDLE");
		notify->changeState(sign_state::IDLE);
		break;
	default:
		break;
	}
}

notifier_config_t loadConfigSafely() {
	auto path = obs_module_config_path("config.json");
	if (not path) return default_notifier_config();

	const auto configFilename = fs::path(path);

	auto configDir = configFilename;
	configDir.remove_filename();

	if (not fs::exists(configDir))
		fs::create_directory(configDir);

	notifier_config_t config;

	auto configInitialized = false;
	if (fs::exists(configFilename)) {
		try {
			auto is = std::ifstream(configFilename);
			auto parser = json::safe::parser(is);
			config = parser.parse<default_notifier_config>();
			configInitialized = true;
		} catch (const std::exception &e) {
			logger_warn("Error while parsing '%s': %s.\n Proceeding with default config", configFilename.c_str(), e.what());
			config = default_notifier_config();
		}
	}

	if (not configInitialized) {
		logger_info("Writing default config to '%s'\nPlease set the secret manually!", path);
		config = default_notifier_config();
		try {
			auto os = std::ofstream(configFilename);
			auto serializer = json::safe::serializer(os);
			serializer.serialize<default_notifier_config>(config);
			os.flush();
			os.close();
		} catch (const std::exception &e) {
			logger_warn("Error while writing default config to '%s': %s.", configFilename.c_str(), e.what());
			config = default_notifier_config();
		}
	}

	bfree(path);

	return config;
}

bool obs_module_load() {

	logger_info("Starting");

	const auto config = loadConfigSafely();
	notify = new notifier(config);

	obs_frontend_add_event_callback(onEvent, nullptr);

	if (const auto e = notify->connect(); e) {
		logger_error("[ASYNC_CONNECT][%s]: %s", e.category().name(), e.message().c_str());
	}

	logger_info("Started");
	
	return true;
}

void obs_module_unload() {
	delete notify;
	logger_info("plugin unloaded");
}
