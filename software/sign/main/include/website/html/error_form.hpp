#include "form.hpp"
#include "input_types.hpp"
#include <algorithm>
#include <system_error>
#include <tuple>
#include <string_view>

#include <esp_log.h>

inline const auto &getErrorForm(const std::error_code &error) {
	static constexpr auto buffer_size = 256;
	static constexpr auto buffer_marker = '`';

	using networking_form = html::form<html::form_field<"Last Error", "e",  input_types::info_text<buffer_size>>{}>;
	static constinit auto form = networking_form::createForm<"", "/reset-error/", "">(
		ztu::string_literal<buffer_size + sizeof('\0')>(buffer_marker)
	);
	static auto index = std::find(form.begin(), form.end(), buffer_marker) - form.begin();
	static constinit auto length = 1;

	static constexpr std::string_view terminator = "...";
	static auto max_msg_len = buffer_size - terminator.size();

	const auto msg = error ? error.message() : std::string("No Errors");
	auto msg_len = msg.length();

	constexpr auto tag = "error_form";
	ESP_LOGI(tag, "msg: '%s'", msg.c_str());
	ESP_LOGI(tag, "from before: '%s'", form.c_str());
	ESP_LOGI(tag, "index: %d length: %d", index, length);

	if (msg_len > max_msg_len) {
		msg_len = buffer_size;
		form.replace(index, length, msg.data(), max_msg_len);
		form.insert(index + max_msg_len, terminator);
	} else {
		form.replace(index, length, msg);
	}

	ESP_LOGI(tag, "from after: '%s'", form.c_str());

	length = msg_len;

	ESP_LOGI(tag, "length after: %d", length);

	return form;
}