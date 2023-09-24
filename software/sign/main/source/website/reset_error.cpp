#include <website/http_handlers.hpp>

esp_err_t reset_error_handler(httpd_req_t *req) {
	sign.error = std::make_error_code(static_cast<std::errc>(0));
	return httpd_resp_send_200(req);
}