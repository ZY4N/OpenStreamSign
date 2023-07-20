#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <cstring>

namespace ztu {

	using std::size_t;
	using ssize_t = std::size_t;

	template<ssize_t N>
		requires (N > 0)
	struct string_literal {

		static constexpr auto maxLength = N - 1;

		std::array<char,N> value{};

		constexpr string_literal() = default;

		constexpr string_literal(const char (&str)[N]) {
			std::copy_n(std::begin(str), N, value.begin());
		}

		template<ssize_t M>
			requires (M < N)
		constexpr string_literal(const char (&str)[M]) {
			std::copy_n(std::begin(str), M, value.begin());
			value[M] = '\0';
		}

		template<typename... Chars>
			requires (sizeof...(Chars) == maxLength)
		constexpr string_literal(Chars... chars) : value{chars..., '\0' } {}

		template<ssize_t M>
		constexpr bool operator==(const string_literal<M> &other) const {
			const auto maxLength = std::min(N,M);
			for (ssize_t i = 0; i < maxLength; i++) {
				if (value[i] != other.value[i]) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] constexpr const char *c_str() const { return value.data(); }

		[[nodiscard]] constexpr std::string_view sv() const { return { value.data() }; }

		[[nodiscard]] constexpr ssize_t size() const {
			ssize_t length = 0;
			while (length < N && value[length] != '\0')
				length++;
			return length;
		}
		
		[[nodiscard]] constexpr auto begin() { return value.begin(); }
		[[nodiscard]] constexpr auto begin() const { return value.begin(); }

		[[nodiscard]] constexpr auto end() { return value.begin() + size(); }
		[[nodiscard]] constexpr auto end() const { return value.begin()+ size(); }


		[[nodiscard]] constexpr unsigned hash() const {

			unsigned prime = 0x1000193;
			unsigned hashed = 0x811c9dc5;

			const auto length = size();
			for (ssize_t i = 0; i < length; i++) {
				hashed = hashed ^ value[i];
				hashed *= prime;
			}

			return hashed;
		}

		template<ssize_t M>
		constexpr auto operator+(const string_literal<M> &other) const {
			return []<class A, class B, ssize_t... IAs, ssize_t... IBs>(
				const A &a, std::index_sequence<IAs...>,
				const B &b, std::index_sequence<IBs...>
			) -> string_literal<sizeof...(IAs) + sizeof...(IBs) + 1> {
				return { a.value[IAs]..., b.value[IBs]... };
			}(
				*this, std::make_index_sequence<N - 1>{},
				other, std::make_index_sequence<M - 1>{}
			);
		}

		template<ssize_t M>
		inline friend std::ostream& operator<<(std::ostream &out, const string_literal<M>& str);
	};

	template<ssize_t M>
	inline std::ostream& operator<<(std::ostream &out, const string_literal<M>& str) {
		return out << str.c_str();
	}

	namespace string_literals {
		template<string_literal Str>
		constexpr auto operator"" _sl() {
			return Str;
		}
	}
}
