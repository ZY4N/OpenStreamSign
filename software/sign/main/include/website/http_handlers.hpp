#pragma once

#include "html/form.hpp"
#include "html/page.hpp"
#include "html/input_types.hpp"
#include "html/error_form.hpp"

#include <domain_logic/sign.hpp>
#include <esp_https_server.h>


inline esp_err_t httpd_resp_send_400(httpd_req_t *req) {
	return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, NULL);
}

inline esp_err_t httpd_resp_send_200(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html");
	return httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
}

esp_err_t style_handler(httpd_req_t *req);
esp_err_t script_handler(httpd_req_t *req);

esp_err_t root_get_handler(httpd_req_t *req);

esp_err_t login_get_handler(httpd_req_t *req);
esp_err_t login_post_handler(httpd_req_t *req);

esp_err_t wifi_get_handler(httpd_req_t *req);
esp_err_t wifi_post_handler(httpd_req_t *req);

esp_err_t networking_get_handler(httpd_req_t *req);
esp_err_t networking_post_handler(httpd_req_t *req);

esp_err_t secret_get_handler(httpd_req_t *req);
esp_err_t secret_post_handler(httpd_req_t *req);

esp_err_t done_get_handler(httpd_req_t *req);
esp_err_t done_post_handler(httpd_req_t *req);

esp_err_t reset_error_handler(httpd_req_t *req);

constexpr httpd_uri_t handlers[]{
	{
		.uri		= "/style.css",
		.method		= HTTP_GET,
		.handler	= style_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/script.js",
		.method		= HTTP_GET,
		.handler	= script_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/",
		.method		= HTTP_GET,
		.handler	= root_get_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/wifi/",
		.method		= HTTP_GET,
		.handler	= wifi_get_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/wifi/",
		.method		= HTTP_POST,
		.handler	= wifi_post_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/networking/",
		.method		= HTTP_GET,
		.handler	= networking_get_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/networking/",
		.method		= HTTP_POST,
		.handler	= networking_post_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/secret/",
		.method		= HTTP_GET,
		.handler	= secret_get_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/secret/",
		.method		= HTTP_POST,
		.handler	= secret_post_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/done/",
		.method		= HTTP_GET,
		.handler	= done_get_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/done/",
		.method		= HTTP_POST,
		.handler	= done_post_handler,
		.user_ctx	= nullptr
	},
	{
		.uri		= "/reset-error/",
		.method		= HTTP_POST,
		.handler	= reset_error_handler,
		.user_ctx	= nullptr
	}
};
