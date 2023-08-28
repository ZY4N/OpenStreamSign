/*
Plugin Name
Copyright (C) 2023 <Developer> admin@zy4n.com

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
#include "app.hpp"

#include <domain_logic/sign_transceiver.hpp>


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

app *app_instance;

static void onEvent(enum obs_frontend_event event, void *){
	switch (event) {
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
	case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED:
		logger_info("STARTED RECORDING");
		app_instance->changeState(sign_state::RECORDING);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_PAUSED:
		logger_info("RECORDING PAUSED");
		app_instance->changeState(sign_state::RECORDING_PAUSED);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
		logger_info("IDLE");
		app_instance->changeState(sign_state::IDLE);
		break;
	default:
		break;
	}
}

bool obs_module_load() {



	logger_info("loading plugin...");

	app_instance = new app;

	if (const auto error = app_instance->start(); error) {
		 logger_error_code("app_start", error);
		 return false;
	}

	obs_frontend_add_event_callback(onEvent, nullptr);

	logger_info("done");
	
	return true;
}

void obs_module_unload() {
	delete app_instance;
	logger_info("plugin unloaded");
}
