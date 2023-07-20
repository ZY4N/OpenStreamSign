#include <website/http_handlers.hpp>

using wifiForm = html::form<
	html::form_field<"SSID:", "ssid", input_types::text<CONFIG_SSID_MAX_LEN>>{},
	html::form_field<"Password:", "pw", input_types::password<CONFIG_PASSWORD_MAX_LEN>>{}
>;

esp_err_t wifi_get_handler(httpd_req_t *req) {
	using namespace ztu::string_literals;

	const auto form = wifiForm::createForm<"Wifi", "/networking/">(""_sl, ""_sl);

	const auto html = html::page::createDefault<"Wifi", "/script.js", "/style.css">(form);

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, html.c_str(), HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}

esp_err_t wifi_post_handler(httpd_req_t *req) {
	httpd_resp_set_type(req, "text/html");

	bool formOk;
	if (const auto values = wifiForm::getPostData(req, formOk); formOk) {
		const auto &ssid = std::get<0>(values);
		const auto &password = std::get<1>(values);
		if (
			ssid.size() > 0 &&
			password.size() > 0
		) {
			sign.storage.set<storage_keys::SSID>(ssid);
			sign.storage.set<storage_keys::PASSWORD>(password);
			httpd_resp_send_200(req);
		} else {
			httpd_resp_send_400(req);
		}
	}

	return ESP_OK;
}
