#include <website/http_handlers.hpp>

esp_err_t root_get_handler(httpd_req_t *req) {
	constexpr auto redirect = "<script>window.location.href='/wifi/'</script>";
	httpd_resp_set_type(req, "text/html");
	return httpd_resp_send(req, redirect, HTTPD_RESP_USE_STRLEN);
}
