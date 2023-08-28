#include <website/http_handlers.hpp>

#include <domain_logic/sign.hpp>

using doneForm = html::form<>;

esp_err_t done_get_handler(httpd_req_t *req) {

	const auto form = doneForm::createForm<"Setup finished!", "about:blank">();

	const auto html = html::page::createDefault<"Done", "/script.js", "/style.css">(form);

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, html.c_str(), HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}

esp_err_t done_post_handler(httpd_req_t *req) {
	ESP_LOGI("DONE_POST", "Setup done");

	httpd_resp_send_200(req);

	// stop setup task
	sign.switch_task.test_and_set();

	return ESP_OK;
}
