#include <website/http_handlers.hpp>

#include <util/base64.hpp>
#include <fill_random.hpp>

#include "esp_log.h"

constexpr auto secretSize = 64 + 32;
constexpr auto secretBase64Size = ztu::base64::encodedSize(secretSize);

using secretForm = html::form<
	html::form_field<"Copy the following Text:", "secret", input_types::text<secretBase64Size>>{}
>;

inline constexpr auto SECRET_TAG = "SECRET_HANDLER";

esp_err_t secret_get_handler(httpd_req_t *req) {
	ztu::string_literal<secretBase64Size + 1> encodedSecret;

	{
		std::array<uint8_t, secretSize> secret;
		fill_random(secret);

		[[maybe_unused]] const auto success = ztu::base64::encode(secret, encodedSecret.value);
		assert(success);
	}

	const auto form = secretForm::createForm<"Secret", "/done/">(encodedSecret);
	const auto html = html::page::createDefault<"Secret", "/script.js", "/style.css">(form);

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, html.c_str(), HTTPD_RESP_USE_STRLEN);

	return ESP_OK;
}

esp_err_t secret_post_handler(httpd_req_t *req) {
	bool formOk;
	if (const auto values = secretForm::getPostData(req, formOk); formOk) {
		const auto &encodedSecret = std::get<0>(values);
		const auto length = encodedSecret.size();
		std::array<uint8_t, secretSize> secret;
		if (
			length == secretBase64Size &&
			ztu::base64::decode({ encodedSecret.begin(), encodedSecret.begin() + length }, secret)
		) {
			sign.storage.set<storage_keys::SECRET>(secret);
			httpd_resp_send_200(req);
		} else {
			httpd_resp_send_400(req);
		}
	}

	return ESP_OK;
}
