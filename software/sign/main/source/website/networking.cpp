#include <website/http_handlers.hpp>

using networkingForm = html::form<
	html::form_field<"IP-Address:", "ip",  input_types::ipv4>{},
	html::form_field<"Netmask:", "nm", input_types::ipv4>{},
	html::form_field<"Gateway:", "gw", input_types::ipv4>{},
	html::form_field<"Port:", "port", input_types::port>{}
>;

esp_err_t networking_get_handler(httpd_req_t *req) {

	ESP_LOGI("NET", "net handler started");

	using ztu::string_literal;
	
	const auto form = networkingForm::createForm<"Networking", "", "/secret/">(
		string_literal(CONFIG_DEFAULT_IP_ADDRESS),
		string_literal(CONFIG_DEFAULT_NETMASK),
		string_literal(CONFIG_DEFAULT_GATEWAY),
		string_literal(CONFIG_DEFAULT_PORT)
	);

	const auto html = html::page::createDefault<"Networking", "/script.js", "/style.css">(
		form + getErrorForm(sign.error)
	);

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, html.c_str(), HTTPD_RESP_USE_STRLEN);

	ESP_LOGI("NET", "net handler terminated");

	return ESP_OK;
}

esp_err_t networking_post_handler(httpd_req_t *req) {

	bool formOk;
	if (const auto values = networkingForm::getPostData(req, formOk); formOk) {
		sign.storage.set<storage_keys::IP_ADDRESS>(std::get<0>(values));
		sign.storage.set<storage_keys::SUBNETMASK>(std::get<1>(values));
		sign.storage.set<storage_keys::GATEWAY	>(std::get<2>(values));
		sign.storage.set<storage_keys::PORT		>(std::get<3>(values));
		return httpd_resp_send_200(req);
	} else {
		return httpd_resp_send_400(req);
	}
}
