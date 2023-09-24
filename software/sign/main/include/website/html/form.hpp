#pragma once

#include <span>
#include <tuple>

#include <util/string_literal.hpp>
#include "esp_https_server.h"

namespace html {

	using ztu::string_literal;

	template<typename T>
	concept input_type = (
		std::is_default_constructible_v<typename T::value_t> &&
		requires(const char *begin, const char *end, typename T::value_t value) {
			T::getAttributes();
			T::parseValue(begin, end, value);
		}
	);

	template<string_literal Title, string_literal Name, input_type Type>
	struct form_field {
		static constexpr auto title = Title;
		static constexpr auto name = Name;
		using type = Type;
	};

	template<form_field... Fields>
	class form {
	public:

		template<
			string_literal Title,
			string_literal PostTo,
			string_literal RedirectTo,
			ssize_t... Ns
		>
			requires (sizeof...(Ns) == sizeof...(Fields))
		static constexpr auto createForm(const string_literal<Ns>&... values);

		static auto getPostData(httpd_req_t *req, bool &success);

	private:
		using value_tuple_t = std::tuple<typename decltype(Fields)::type::value_t...>;

		template<class Field, ssize_t N>
		static constexpr auto createField(const string_literal<N> &value);

		static auto parsePostBody(std::span<char> buffer, value_tuple_t &values);

	};

	template<form_field... Fields>
	template<class Field, ssize_t N>
	constexpr auto form<Fields...>::createField(const string_literal<N> &value) {
		using namespace ztu::string_literals;

		constexpr auto labelHtml = [&]() {
			if constexpr (Field::title.length() > 0) {
				return "<label for='"_sl + Field::name + "'>"_sl + Field::title + "</label><br>"_sl;
			} else {
				return ""_sl;
			}
		}();

		constexpr auto beforeValue = labelHtml +
			"<input name='"_sl + Field::name + "'"_sl + Field::type::getAttributes() + " value='"_sl;
		
		return beforeValue + value + "'><br>"_sl;
	}

	template<form_field... Fields>
	template<
		string_literal Title,
		string_literal PostTo,
		string_literal RedirectTo,
		ssize_t... Ns
	>
		requires (sizeof...(Ns) == sizeof...(Fields))
	constexpr auto form<Fields...>::createForm(
		const string_literal<Ns>&... values
	) {
		using namespace ztu::string_literals;

		constexpr auto title = []() {
			if constexpr (Title.length() > 0) {
				return "<h1>"_sl + Title + "</h1>"_sl;
			} else {
				return ""_sl;
			}
		}();

		constexpr auto beforeFields = (
			"<div id='input-box' class='content-container'>"_sl + title +
			"<form action='"_sl + PostTo + "'redirect='"_sl + RedirectTo +
			"'method='post'>"_sl
		);

		constexpr auto afterFields = "<input type=submit value=Ok></form></div>"_sl;
		
		if constexpr (sizeof...(values) > 0) {
			return beforeFields + (createField<decltype(Fields)>(values) + ...) + afterFields;
		} else {
			return beforeFields + afterFields;
		}
	}

	template<form_field... Fields>
	auto form<Fields...>::getPostData(httpd_req_t *req, bool &success) {
		constexpr auto BufferSize = ((
			decltype(decltype(Fields)::name)::max_size + 1 +
			decltype(Fields)::type::maxBytes) + ...
		) + sizeof...(Fields) - 1;
		
		std::array<char, BufferSize> buffer;

		value_tuple_t values;

		if (req->content_len <= buffer.size() && httpd_req_recv(req, buffer.data(), req->content_len) > 0) {
			success = parsePostBody({ buffer.begin(), buffer.begin() + req->content_len }, values);
		} else {
			printf("Could not parse buffer %d %d\n", req->content_len, buffer.size());
			success = false;
		}

		return values;
	}

	template<form_field... Fields>
	auto form<Fields...>::parsePostBody(std::span<char> buffer, value_tuple_t &values) {
		
		constexpr char equalChar = 255;
		constexpr char andChar = 254;

		auto bufferEnd = buffer.end();
		const auto parseKeyValuePair = [&]<typename Field>(auto &value) {
			const auto sv = std::string_view(buffer.begin(), bufferEnd);

			constexpr auto &key = Field::name;

			const auto keyIndex = sv.find(key.c_str());
			if (keyIndex == std::string::npos)
				return false;

			const auto keyBegin = buffer.begin() + keyIndex;
			if ((bufferEnd - keyBegin) < key.max_length + 1)
				return false;

			const auto keyEnd = keyBegin + key.max_length;
			if (*keyEnd != equalChar)
				return false;

			const auto valueBegin = keyEnd + 1;
			const auto valueEnd = std::find(valueBegin, bufferEnd, andChar);

			const auto length = valueEnd - valueBegin;
			using valueType = typename Field::type;
			if (length > valueType::maxBytes)
				return false;

			valueType::parseValue(&(*valueBegin), &(*valueEnd), value);

			const auto fullLength = valueEnd - keyBegin;
			std::move(valueEnd, bufferEnd, keyBegin);
			bufferEnd -= fullLength;
		
			return true;
		};

	
		bool success = true;
		std::apply(
			[&](auto&... value) {
				success = (parseKeyValuePair.template operator()<decltype(Fields)>(value) && ... );
			},
			values
		);

		return success;
	}
}
