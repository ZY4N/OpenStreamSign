#pragma once

#include <utility>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <cassert>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <array>
#include <tuple>
#include <bitset>
#include <vector>
#include <variant>
#include <limits>
#include <fstream>
#include <ostream>
#include <format>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <stdexcept>

#include <iostream>


#ifndef CONSTEXPR_FOR
#define CONSTEXPR_FOR

namespace constexpr_for {

	template<typename... Types>
	inline constexpr bool types(auto &&f) {
		return (f.template operator()<Types>() || ...);
	}

	template<auto... Values>
	inline constexpr bool values(auto &&f) {
		return (f.template operator()<Values>() || ...);
	}

	template<typename... Args>
	inline constexpr bool arguments(auto &&f, Args&&... args) {
		return (f(std::forward<Args>(args)) || ...);
	}

	template <auto Size>
	inline constexpr bool index(auto&& f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices>() || ...);
		}(std::make_index_sequence<Size>());
	}

	template<typename... Types>
	inline constexpr bool indexed_types(auto &&f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices, Types>() || ...);
		}(std::make_index_sequence<sizeof...(Types)>());
	}

	template<auto... Values>
	inline constexpr bool indexed_values(auto &&f) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices, Values>() || ...);
		}(std::make_index_sequence<sizeof...(Values)>());
	}

	template<typename... Args>
	inline constexpr bool indexed_arguments(auto &&f, Args&&... args) {
		return [&]<auto... Indices>(std::index_sequence<Indices...>) {
			return (f.template operator()<Indices>(std::forward<Args>(args)) || ...);
		}(std::make_index_sequence<sizeof...(Args)>());
	}


	template<class T>
    struct pack {};

    template<template<typename...> class T, typename... Ts>
    struct pack<T<Ts...>> {
	private:
        [[maybe_unused]] bool value;
	public:
        inline constexpr pack(auto&& f) {
            value = (f.template operator()<Ts>() || ...);
        }
        inline constexpr operator bool() {
            return value;
        }
    };


	template<class T>
    struct indexed_pack {};

    template<template<typename...> class T, typename... Ts>
    struct indexed_pack<T<Ts...>> {
	private:
        [[maybe_unused]] bool value;
	public:
        inline constexpr indexed_pack(auto&& f) {
            [&]<auto... Is>(std::index_sequence<Is...>) {
				value = (f.template operator()<Is, Ts>() || ...);
			}(std::make_index_sequence<sizeof...(Ts)>{});
        }
        inline constexpr operator bool() {
            return value;
        }
    };
}

template<class T, typename... Ts>
struct prepend_types {};

template<template<typename...> class T, typename... Ts, typename... Us>
struct prepend_types<T<Ts...>, Us...> {
	using type = T<Us..., Ts...>;
};

template<class T, typename... Ts>
using prepend_types_t = prepend_types<T, Ts...>::type;

template<class T, typename... Ts>
struct append_types {};

template<template<typename...> class T, typename... Ts, typename... Us>
struct append_types<T<Ts...>, Us...> {
	using type = T<Ts..., Us...>;
};

#endif


#ifndef STRING_LITERAL
#define STRING_LITERAL

using std::size_t;
using ssize_t = std::make_signed_t<size_t>;

template<ssize_t N>
	requires (N > 0)
struct string_literal {
	static constexpr auto maxLength = N - 1;

	std::array<char,N> value{};

	constexpr string_literal() = default;

	constexpr string_literal(const char (&str)[N]);

	template<ssize_t M> requires (M > 0 and M <= N)
	constexpr string_literal(const char (&str)[M]);

	template<typename... Chars> requires (sizeof...(Chars) == maxLength)
	constexpr string_literal(Chars... chars) : value{chars..., '\0' } {}

	template<ssize_t M>
	constexpr bool operator==(const string_literal<M> &other) const;

	[[nodiscard]] constexpr const char *c_str() const;

	[[nodiscard]] constexpr ssize_t size() const;

	[[nodiscard]] constexpr auto begin();
	[[nodiscard]] constexpr auto begin() const;

	[[nodiscard]] constexpr auto end();
	[[nodiscard]] constexpr auto end() const;

	constexpr decltype(auto) operator[](size_t index) const {
		return value[index];
	}

	constexpr decltype(auto) operator[](size_t index) {
		return value[index];
	}

	template<ssize_t M>
	constexpr auto operator+(const string_literal<M> &other) const;

	template<ssize_t M>
	inline friend std::ostream& operator<<(std::ostream &out, const string_literal<M>& str);
};

template<ssize_t N>
	requires (N > 0)
constexpr string_literal<N>::string_literal(const char (&str)[N]) {
	std::copy_n(std::begin(str), N, value.begin());
}

template<ssize_t N> requires (N > 0)
template<ssize_t M> requires (M > 0 and M <= N)
constexpr string_literal<N>::string_literal(const char (&str)[M]) {
	std::copy_n(std::begin(str), M, value.begin());
	value[M-1] = '\0';
}

template<ssize_t N>
	requires (N > 0)
template<ssize_t M>
constexpr bool string_literal<N>::operator==(const string_literal<M> &other) const {
	const auto length = std::min(N,M);
	for (ssize_t i = 0; i < length; i++) {
		if (value[i] != other.value[i]) {
			return false;
		}
	}
	return true;
}

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr const char *string_literal<N>::c_str() const { return value.data(); }

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr ssize_t string_literal<N>::size() const {
	ssize_t length = 0;
	while (length < N && value[length] != '\0')
		length++;
	return length;
}

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr auto string_literal<N>::begin() { return value.begin(); }

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr auto string_literal<N>::begin() const { return value.begin(); }

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr auto string_literal<N>::end() { return value.begin() + size(); }

template<ssize_t N> requires (N > 0)
[[nodiscard]] constexpr auto string_literal<N>::end() const { return value.begin() + size(); }

template<ssize_t N> requires (N > 0)
template<ssize_t M>
constexpr auto string_literal<N>::operator+(const string_literal<M> &other) const {
	return []<class A, class B, ssize_t... IAs, ssize_t... IBs>(
		const A &a, std::index_sequence<IAs...>,
		const B &b, std::index_sequence<IBs...>
	) -> string_literal<sizeof...(IAs) + sizeof...(IBs) + 1> {
		return {a.dynamic_value[IAs]..., b.dynamic_value[IBs]... };
	}(
		*this, std::make_index_sequence<N - 1>{},
		other, std::make_index_sequence<M - 1>{}
	);
}

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

#endif

#ifndef JSON_TRANSCODING
#define JSON_TRANSCODING

namespace json::transcoding {

	template<class T, typename Enum>
	concept token = (
		std::same_as<Enum, typename T::enum_t> &&
		requires(const T::value_t value, std::vector<char> buffer, std::istream is, std::ostream os) {
			{ T::beginsWith(is)		} -> std::same_as<bool>;
			{ T::read(is, buffer)	} -> std::same_as<typename T::value_t>; // std::optional<...>
			{ T::write(os, value)	} -> std::same_as<void>;
			{ T::name()				} -> std::same_as<std::string_view>;
			{ T::type()				} -> std::same_as<typename T::enum_t>;
		}
	);

	namespace tokens {
		namespace detail {
			template<typename Enum, typename ValueType, Enum Type, string_literal Name>
			struct token_base {
				using enum_t = Enum;
				using value_t = ValueType;

				constexpr static inline Enum type() {
					return Type;
				}

				constexpr static inline std::string_view name() {
					return { Name.c_str() };
				}
			};
		}

		enum class type : unsigned char {
			END,
			LBRACE, RBRACE,
			COLON, COMMA,
			STRING, NUMBER, BOOLEAN,
			LBRACKET, RBRACKET,
			UNDEFINED,
		};

		namespace detail {

			template<type Type, string_literal Name, typename ValueType = std::monostate>
			using json_token_base = detail::token_base<type, ValueType, Type, Name>;

			template<type Type, string_literal Name, string_literal... Spellings> requires (sizeof...(Spellings) <
																							255) // && all Spellings must begin with a different char because ll1
			struct json_basic_token : public json_token_base<Type, Name, uint8_t> {
				using base_t = json_token_base<Type, Name, uint8_t>;
				using typename base_t::value_t;

				inline static bool beginsWith(std::istream &is) {
					const auto c = is.peek();
					return constexpr_for::values<Spellings...>(
						[&]<auto Spelling>() {
							return c == Spelling[0];
						}
					);
				}

				inline static value_t read(std::istream &is, std::vector<char> &) {
					value_t index = 255;
					const auto foundSpelling = constexpr_for::indexed_values<Spellings...>(
						[&]<auto Index, auto Spelling>() {
							for (const auto &c: Spelling) {
								if (is.peek() != c) {
									return false;
								}
								is.get();
							}
							index = Index;
							return true;
						}
					);
					if (not foundSpelling) {
						throw std::runtime_error("wrong spelling");
					}
					return index;
				}

				inline static void write(std::ostream &os, const value_t &index) {
					constexpr_for::indexed_values<Spellings...>(
						[&]<auto Index, auto Spelling>() {
							if (index == Index) {
								os.write(Spelling.c_str(), Spelling.size());
								return true;
							}
							return false;
						}
					);
				}
			};


			template<type Type, string_literal Name, char BeginChar, char EndChar>
			struct json_string_literal : public json_token_base<Type, Name, std::string_view> {
				using base_t = json_token_base<Type, Name, std::string_view>;
				using typename base_t::value_t;

				inline static bool beginsWith(std::istream &is) {
					return is.peek() == BeginChar;
				}

				inline static value_t read(std::istream &is, std::vector<char> &buffer) {
					[[maybe_unused]] const auto first = is.get();
					assert(first == BeginChar);
					assert(buffer.empty());
					char c;
					while ((c = static_cast<char>(is.get())) != EndChar) {
						if (c == '\n') [[unlikely]] {
							is.setstate(std::ios_base::failbit);
							break;
						}
						buffer.push_back(c);
					};
					return { buffer.data(), buffer.data() + buffer.size() };
				}

				inline static void write(std::ostream &os, const value_t &value) {
					os << BeginChar << value << EndChar;
				}
			};

			template<type Type, string_literal Name>
			struct json_number_literal : public json_token_base<Type, Name, double> {
				using base_t = json_token_base<Type, Name, double>;
				using typename base_t::value_t;

				inline static bool beginsWith(std::istream &is) {
					const auto c = is.peek();
					return std::isdigit(c) or c == '-' or c == '+';
				}

				inline static value_t read(std::istream &is, std::vector<char> &) {
					value_t d;
					is >> d;
					return d;
				}

				inline static void write(std::ostream &os, const value_t &value) {
					os << value;
				}
			};

			template<type Type, string_literal Name>
			struct json_end_token : public json_token_base<Type, Name> {
				using base_t = json_token_base<Type, Name>;
				using typename base_t::value_t;

				inline static bool beginsWith(std::istream &is) {
					return is.eof();
				}

				value_t static read(std::istream &, std::vector<char> &) {
					return {};
				}

				inline static void write(std::ostream &os, const value_t &) {
					os.flush();
				}
			};
		}

		using left_brace = detail::json_basic_token<type::LBRACE, "left-brace", "{">;
		using right_brace = detail::json_basic_token<type::RBRACE, "right-brace", "}">;
		using left_bracket = detail::json_basic_token<type::LBRACKET, "left-bracket", "[">;
		using right_bracket = detail::json_basic_token<type::RBRACKET, "right-bracket", "]">;
		using colon = detail::json_basic_token<type::COLON, "colon", ":">;
		using comma = detail::json_basic_token<type::COMMA, "comma", ",">;
		using null = detail::json_basic_token<type::UNDEFINED, "null", "null">;
		using boolean = detail::json_basic_token<type::BOOLEAN, "boolean", "false", "true">;
		using string = detail::json_string_literal<type::STRING, "string", '"', '"'>;
		using number = detail::json_number_literal<type::NUMBER, "number">;
		using end = detail::json_end_token<type::END, "end">;

	}



	struct source_location {
		size_t row, column;

		inline friend std::ostream &operator<<(std::ostream &os, const source_location &location);
	};

	std::ostream &operator<<(std::ostream &os, const source_location &location) {
		return os << '(' << location.row << ", " << location.column << ')';
	}


	using namespace std::literals;

	template<typename Enum, token <Enum>... Tokens>
		// requires Tokens::type() == index++ && ...
	class token_instance {
	private:
		using enum_t = std::underlying_type_t<Enum>;
		inline constexpr static auto names = std::array{Tokens::name()..., "unknown"sv};

	public:
		using value_t = std::variant<typename Tokens::value_t...>;

		template<typename... Args>
		inline constexpr token_instance(Args &&... args)
			: m_value(std::forward<Args>(args)...) {}

		template<Enum t, typename... Args>
		inline constexpr static decltype(auto) make(Args &&... args) {
			constexpr auto index = static_cast<size_t>(static_cast<enum_t>(t));
			return token_instance(std::in_place_index<index>, std::forward<Args>(args)...);
		}

		inline constexpr static std::string_view nameOf(Enum t) {
			const auto index = static_cast<size_t>(static_cast<enum_t>(t));
			return names[std::min(index, sizeof...(Tokens))];
		}

		template<Enum t, typename... Args>
		inline void set(Args &&... args) {
			constexpr auto index = static_cast<size_t>(static_cast<enum_t>(t));
			m_value.template emplace<index>(std::forward<Args>(args)...);
		}

		template<Enum t>
		inline decltype(auto) get() {
			constexpr auto index = static_cast<size_t>(static_cast<enum_t>(t));
			return std::get<index>(m_value);
		}

		template<Enum t>
		inline decltype(auto) get() const {
			constexpr auto index = static_cast<size_t>(static_cast<enum_t>(t));
			return std::get<index>(m_value);
		}

		[[nodiscard]] inline Enum type() const {
			return static_cast<Enum>(static_cast<enum_t>(m_value.index()));
		}

	private:
		value_t m_value;
	};

	using json_token_t = token_instance<tokens::type,
		tokens::end,
		tokens::left_brace,
		tokens::right_brace,
		tokens::comma,
		tokens::colon,
		tokens::string,
		tokens::number,
		tokens::boolean,
		tokens::left_bracket,
		tokens::right_bracket,
		tokens::null
	>;


	template<typename Enum, token <Enum>... Tokens>
	class tokenizer {
	public:
		using token_t = token_instance<Enum, Tokens...>;

		inline tokenizer() = default;

		inline tokenizer(std::istream &is) : m_is{&is} {}

		inline void setIs(std::istream &is) {
			m_is = &is;
			m_location = {1, 1};
		}

		inline decltype(auto) operator>>(std::tuple<token_t &, source_location &> dst) {

			auto &[token, location] = dst;

			while (not m_is->eof() and std::isspace(m_is->peek())) {
				accept();
			}

			// The guaranteed lifetime of the previous string token ends here
			m_buffer.clear();

			location = m_location;
			const auto beforePos = m_is->tellg();

			const auto foundToken = constexpr_for::types<Tokens...>([&]<class Token>() {
				if (Token::beginsWith(*m_is)) {
					token.template set<Token::type()>(
						Token::read(*m_is, m_buffer)
					);
					return true;
				}
				return false;
			});

			const auto afterPos = m_is->tellg();
			if (afterPos == -1 && not m_is->eof()) {
				throw std::runtime_error(
					std::format(
						"Error while parsing {} token starting at position ({}, {})",
						token_t::nameOf(token.type()),
						m_location.row,
						m_location.column
					)
				);
			}

			// assuming there are no linebreaks within a token
			m_location.column += afterPos - beforePos;

			if (not foundToken) {
				using namespace std::string_literals;
				throw std::runtime_error(
					std::format(
						"Unexpected character '{}' at position ({}, {})",
						static_cast<char>(m_is->peek()),
						m_location.row,
						m_location.column
					)
				);
			}

			return *this;
		}

		inline char ignore(std::span<const char> options) {
			while (true) {
				const auto c = m_is->peek();
				if (not std::isspace(c)) {
					for (const auto &option: options) {
						if (c == option) {
							accept();
							return c;
						}
					}
				}
				accept();
			}
		}

		inline void accept() {
			++m_location.column;
			if (m_is->get() == '\n') {
				++m_location.row;
				m_location.column = 1;
			}
		}

	private:
		source_location m_location{1, 1};
		std::istream *m_is;
		std::vector<char> m_buffer;
	};


	enum class detokenizer_instructions {
		space, tab, indent, outdent, clear_indent, newline
	};

	struct detokenizer_format {
		std::string_view m_space, m_tab, m_newline;
	};

	namespace detokenizer_formats {
		constexpr detokenizer_format pretty_tab{
			.m_space{ " " },
			.m_tab{ "\t" },
			.m_newline{ "\n" },
		};
		constexpr detokenizer_format pretty_space{
			.m_space{ " " },
			.m_tab{ " " },
			.m_newline{ "\n" },
		};
		constexpr detokenizer_format minimal{
			.m_space{ "" },
			.m_tab{ "" },
			.m_newline{ "" },
		};
	}

	template<typename Enum, token <Enum>... Tokens>
	class detokenizer {
	public:

		using token_t = token_instance<Enum, Tokens...>;

		inline detokenizer(const detokenizer_format &fmt = detokenizer_formats::pretty_tab)
			: m_format{fmt} {};

		inline detokenizer(std::ostream &os, const detokenizer_format &fmt = detokenizer_formats::pretty_tab)
			: m_os{&os}, m_format{fmt} {}

		inline void setOs(std::ostream &os) {
			m_os = &os;
		}

		inline size_t depth() {
			if (m_format.m_tab.empty()) {
				return 0;
			} else {
				return m_indent.size() / m_format.m_tab.size();
			}
		}

		inline decltype(auto) operator<<(const detokenizer_instructions &instr) {
			switch (instr) {
				using
				enum detokenizer_instructions;
				case space: {
					(*m_os) << m_format.m_space;
					break;
				}
				case tab: {
					(*m_os) << m_format.m_tab;
					break;
				}
				case indent: {
					m_indent += m_format.m_tab;
					break;
				}
				case outdent: {
					const auto length = (
						static_cast<long long>(m_indent.length()) -
						static_cast<long long>(m_format.m_tab.size())
					);
					m_indent.resize(std::max(0LL, length));
					break;
				}
				case clear_indent: {
					m_indent.clear();
					break;
				}
				case newline: {
					(*m_os) << m_format.m_newline << m_indent;
					break;
				}
				default: {
					throw std::runtime_error(std::format("Unsupported tkn '{}'", int(instr)));
				}
			}
			return *this;
		}

		inline decltype(auto) operator<<(const token_t &token) {

			const auto type = token.type();
			const auto foundToken = constexpr_for::types<Tokens...>([&]<class Token>() {
				if (Token::type() == type) {
					Token::write(*m_os, token.template get<Token::type()>());
					return true;
				}
				return false;
			});

			if (not foundToken) {
				using namespace std::string_literals;
				throw std::runtime_error(
					std::format(
						"Unknown token '{}'",
						token_t::nameOf(type)
					)
				);
			}

			return *this;
		}

	private:
		std::ostream *m_os{ nullptr };
		std::string m_indent{};
		detokenizer_format m_format;
	};

	template<typename Enum, token <Enum> ... Tokens>
	class parser {
	public:
		using token_t = token_instance<Enum, Tokens...>;

		inline parser() : tokens() {};

		inline parser(std::istream &is) : tokens(is) {
			accept();
		};

		inline void setIs(std::istream &is) {
			tokens.setIs(is);
			accept();
		}

	protected:
		inline void accept() {
			tokens >> std::tie(token, location);
		}

		inline token_t accept(Enum type) {
			if (token.type() != type) {
				using namespace std::string_literals;
				throw std::runtime_error(
					std::format(
						"Expected token '{}' but got token '{}' at position ({}, {})",
						token_t::nameOf(type),
						token_t::nameOf(token.type()),
						location.row,
						location.column
					)
				);
			}
			auto oldToken = std::move(token);
			if (oldToken.type() != tokens::type::END) {
				accept();
			}
			return oldToken;
		}

		tokenizer<Enum, Tokens...> tokens;
		token_t token;
		source_location location;
	};

	template<typename Enum, token <Enum> ... Tokens>
	class serializer {
	public:
		using token_t = token_instance<Enum, Tokens...>;

		template<typename... Args>
		inline serializer(Args &&... args) : tokens{std::forward<Args>(args)...} {}

		inline void setOs(std::ostream &os) {
			tokens.setOs(os);
		}

	protected:
		using
		enum tokens::type;

		inline constexpr static auto
			lbrace = json_token_t::make<LBRACE>(),
			rbrace = json_token_t::make<RBRACE>(),
			lbracket = json_token_t::make<LBRACKET>(),
			rbracket = json_token_t::make<RBRACKET>(),
			colon = json_token_t::make<COLON>(),
			comma = json_token_t::make<COMMA>(),
			null = json_token_t::make<UNDEFINED>(),
			end = json_token_t::make<END>();

		inline constexpr static json_token_t boolean(const bool b) {
			return json_token_t::make<BOOLEAN>(b);
		}

		inline constexpr static json_token_t string(const std::string_view &s) {
			return json_token_t::make<STRING>(s);
		}

		inline constexpr static json_token_t number(const double d) {
			return json_token_t::make<NUMBER>(d);
		}

		detokenizer<Enum, Tokens...> tokens;
	};
}

namespace json {
	using format = transcoding::detokenizer_format;
	namespace formats = transcoding::detokenizer_formats;

}

#endif

#ifndef STRING_INDEXER
#define STRING_INDEXER

template<size_t NumKeys>
class string_indexer {
private:
	struct index_type {
		unsigned int hash;
		size_t index;

		constexpr inline auto operator<=>(const index_type &other) const {
			return hash <=> other.hash;
		}

		constexpr inline auto operator==(const index_type &other) const {
			return hash == other.hash;
		}

		constexpr inline auto operator<=>(const unsigned otherHash) const {
			return hash <=> otherHash;
		}
	};

	[[nodiscard]] inline constexpr static unsigned hash(std::span<const char> str);

public:
	template<typename... Ts>
		requires (sizeof...(Ts) == NumKeys)
	consteval string_indexer(const Ts&... keys) noexcept;

	[[nodiscard]] inline constexpr std::optional<size_t> indexOf(std::span<const char> str) const;

	[[nodiscard]] inline constexpr std::optional<std::string_view> nameOf(size_t index) const;

private:
	std::array<index_type, NumKeys> m_lookup{};
	std::array<std::string_view, NumKeys> m_keys{};
};

template<size_t NumKeys>
[[nodiscard]] inline constexpr unsigned string_indexer<NumKeys>::hash(std::span<const char> str) {

	unsigned prime = 0x1000193;
	unsigned hashed = 0x811c9dc5;

	for (const auto &c : str) {
		hashed = hashed ^ c;
		hashed *= prime;
	}

	return hashed;
}

template<size_t NumKeys>
template<typename... Ts> requires (sizeof...(Ts) == NumKeys)
consteval string_indexer<NumKeys>::string_indexer(const Ts&... keys) noexcept {

	constexpr_for::indexed_arguments([&]<auto Index>(const auto &key) {
		// Since std::string_view does only truncate the '\0' of strings in the 'const char*' constructor
		// and does not deem otherwise equal views of truncated and untruncated strings equal,
		// all strings need to be truncated before constructing the view.
		const auto begin = std::begin(key), end = std::end(key);
		m_keys[Index] = { begin, std::find(begin, end, '\0') };
		m_lookup[Index] = { hash(m_keys[Index] ), Index };
		return false;
	}, keys...);

	std::sort(m_lookup.begin(), m_lookup.end());

	auto it = m_lookup.begin();
	while ((it = std::adjacent_find(it, m_lookup.end())) != m_lookup.end()) {
		const auto match = it->hash;
		for (auto itA = it + 1; itA != m_lookup.end() && itA->hash == match; itA++) {
			const auto &keyA = m_keys[itA->index];
			for (auto itB = it; itB != itA; itB++) {
				const auto &keyB = m_keys[itB->index];
				if (keyA == keyB) {
					throw std::logic_error("Duplicate keys");
				}
			}
		}
	}
}

template<size_t NumKeys>
[[nodiscard]] inline constexpr std::optional<size_t> string_indexer<NumKeys>::indexOf(std::span<const char> str) const {
	const auto sv = std::string_view(str.begin(), std::find(str.begin(), str.end(), '\0')); // TODO don't do this, just don't

	const auto hashed = hash(sv);
	const auto it = std::lower_bound(m_lookup.begin(), m_lookup.end(), hashed);

	if (it == m_lookup.end() or hashed != it->hash)
		return std::nullopt;

	do [[unlikely]] {
		const auto candidateIndex = it->index;
		if (m_keys[candidateIndex] == sv) [[likely]] {
			return candidateIndex;
		}
	} while (it < m_lookup.end() && it->hash == hashed);

	return std::nullopt;
}

template<size_t NumKeys>
[[nodiscard]] inline constexpr std::optional<std::string_view> string_indexer<NumKeys>::nameOf(size_t index) const {
	if (index < NumKeys) {
		return m_keys[index];
	} else {
		return std::nullopt;
	}
}

#endif

#ifndef NAMED_ENUM
#define NAMED_ENUM

template<string_literal... Values>
class named_enum {
	private:
		static constexpr auto indexer = string_indexer<sizeof...(Values)>(Values...);

	public:
		inline constexpr named_enum() = default;

		inline constexpr named_enum(size_t newIndex)
			: m_index{ newIndex } {};

		inline constexpr static size_t indexOf(std::string_view name) {
			return indexer.indexOf(name);
		}

		inline static constexpr size_t size() {
			return sizeof...(Values);
		}

		inline constexpr size_t index() const {
			return m_index;
		}

		inline std::optional<std::string_view> name() const {
			return indexer.nameOf(m_index);
		}

	private:
		size_t m_index{ 0 };
};

#endif

#ifndef NAMED_TYPE
#define NAMED_TYPE
template<string_literal Key, typename Type>
struct named_type {
	static constexpr auto key = Key;
	using type = Type;
};
#endif

#ifndef NAMED_TUPLE
#define NAMED_TUPLE

template<named_type... Elements>
class named_tuple {
	private:
		static constexpr auto indexer = string_indexer<sizeof...(Elements)>(decltype(Elements)::key...);

		template<string_literal Key, typename Type>
		struct entry_t {
			static constexpr auto key = Key;
			Type value;
		};

		using entry_tuple_t = std::tuple<
		    entry_t<decltype(Elements)::key, typename decltype(Elements)::type>...
		>;

		using value_tuple_t = std::tuple<typename decltype(Elements)::type...>;

	public:
		inline constexpr named_tuple() = default;

		template<typename... Ts>
		inline constexpr named_tuple(const Ts&... args)
			: elements{ { args }... } {}

		inline constexpr static auto indexOf(std::string_view name) {
			return indexer.indexOf(name);
		}

		inline constexpr static auto nameOf(size_t index) {
			return indexer.nameOf(index);
		}

		inline static constexpr size_t size() {
			return sizeof...(Elements);
		}

		template<size_t Index>
		inline auto &get() {
			return std::get<Index>(elements).value;
		}

		template<size_t Index>
		inline const auto &get() const {
			return std::get<Index>(elements).value;
		}

		template<string_literal Name>
		inline auto &get() {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			return get<*index_opt>();
		}

		template<string_literal Name>
		inline const auto &get() const {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			return get<*index_opt>();
		}

		template<class F>
		inline void apply(F &&visitor) {
			std::apply(visitor, elements);
		}

		template<class F>
		inline void apply(F &&visitor) const {
			std::apply(visitor, elements);
		}

		inline value_tuple_t &value() {
			return *reinterpret_cast<value_tuple_t*>(&elements);
		}

		inline const value_tuple_t &value() const {
			return *reinterpret_cast<const value_tuple_t*>(&elements);
		}

	entry_tuple_t elements{};
};

#endif

#ifndef NAMED_VARIANT
#define NAMED_VARIANT

template<string_literal Name>
struct in_place_name_t {
	static constexpr auto name = Name;
};

template<string_literal Name>
static constexpr auto in_place_name = in_place_name_t<Name>{};

struct in_place_dynamic_index {
	size_t value;
};

template<named_type... Alternatives>
class named_variant {
	private:

		static constexpr auto indexer = string_indexer<sizeof...(Alternatives)>(
			decltype(Alternatives)::key...
		);

		template<string_literal Key, typename Type>
		struct entry_t {

			template<typename... Args>
			constexpr entry_t(Args&&... args)
				: value{ std::forward<Args>(args)... } {}

			static constexpr auto key = Key;
			static constexpr auto index_opt = indexer.indexOf(Key);
			static_assert(index_opt);
			static constexpr auto index = *index_opt;

			Type value;
		};

		using entry_variant_t = std::variant<
		    entry_t<decltype(Alternatives)::key, typename decltype(Alternatives)::type>...
		>;

		using value_variant_t = std::variant<typename decltype(Alternatives)::type...>;

	public:
		constexpr inline named_variant() = default;

		template<typename... Args>
		constexpr explicit inline named_variant(Args&&... args)
			: alternatives(std::forward<Args>(args)...) {}

		template<string_literal Name, typename... Args>
		constexpr explicit inline named_variant(in_place_name_t<Name>, Args&&... args)
			: alternatives(std::in_place_index<*indexer.indexOf(Name)>, std::forward<Args>(args)...) {}

		template<typename... Args>
		constexpr explicit inline named_variant(in_place_dynamic_index in_place_index, Args&&... args) {
			constexpr_for::index<sizeof...(Alternatives)>([&]<auto Index>() {
				if (in_place_index.value == Index) {
					alternatives.template emplace<Index>(std::forward<Args>(args)...);
					return true;
				}
				return false;
			});
		}

		template<string_literal Name, typename... Args>
		inline static constexpr decltype(auto) make(Args&&... args) {
			named_variant<Alternatives...> variantValue;
			variantValue.emplace<Name>(std::forward<Args>(args)...);
			return variantValue;
		}

		inline constexpr static auto indexOf(std::span<const char> name) {
			return indexer.indexOf(name);
		}

		inline constexpr static auto nameOf(size_t index) {
			return indexer.nameOf(index);
		}

		[[nodiscard]] inline constexpr size_t index() const {
			return alternatives.index();
		}

		[[nodiscard]] inline static constexpr size_t size() {
			return sizeof...(Alternatives);
		}

		template<string_literal Name, typename... Args>
		inline constexpr void emplace(Args&&... args) {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			alternatives.template emplace<*index_opt>(std::forward<Args>(args)...);
		}

		template<size_t Index>
		inline auto &get() {
			return std::get<Index>(alternatives).value;
		}

		template<size_t Index>
		inline const auto &get() const {
			return std::get<Index>(alternatives).value;
		}

		template<string_literal Name>
		inline auto &get() {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			return get<*index_opt>();
		}

		template<string_literal Name>
		inline const auto &get() const {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			return get<*index_opt>();
		}

		template<string_literal Name>
		inline auto *get_if() {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			const auto ptr = std::get_if<*index_opt>(&alternatives);
			return ptr ? &ptr->value : nullptr;
		}

		template<string_literal Name>
		inline const auto *get_if() const {
			constexpr auto index_opt = indexer.indexOf(Name);
			static_assert(index_opt, "Unknown key");
			const auto ptr = std::get_if<*index_opt>(&alternatives);
			return ptr ? &ptr->value : nullptr;
		}

		template<class F>
		inline decltype(auto) visit(F &&visitor) {
			return std::visit([&visitor](auto &value) {
				visitor(value);
			}, alternatives);
		}

		template<class F>
		inline decltype(auto) visit(F &&visitor) const {
			return std::visit([&visitor](const auto& value) {
				visitor(value);
			}, alternatives);
		}

		inline value_variant_t &value() {
			return *reinterpret_cast<value_variant_t*>(&alternatives);
		}

		inline const value_variant_t &value() const {
			return *reinterpret_cast<const value_variant_t*>(&alternatives);
		}

	private:
		entry_variant_t alternatives{};
};

#endif


namespace json::safe {

	using boolean_t = bool;
	using number_t = double;
	using string_t = std::string;

	template<typename ElementType>
	using array_t = std::vector<ElementType>;

	template<named_type... Elements>
	using object_t = named_tuple<Elements...>;

	template<named_type... Alternatives>
	using varaint_t = named_variant<Alternatives...>;


	namespace default_values {

		enum class type {
			BOOLEAN, NUMBER, STRING,
			ARRAY, OBJECT, VARIANT, ADAPTER,
			INVALID
		};

		using enum type;

		//----------------------[ default type to enum ]----------------------//

		template<type EnumType>
		struct type_container {
			static constexpr auto value = EnumType;
		};

		template<typename T>
		struct registerDefaultValue {
			static constexpr auto value = INVALID;
		};

		template<typename T>
		constexpr auto typeOfDefault(const T &) {
			return registerDefaultValue<std::remove_cvref_t<T>>::value;
		}

		//----------------------[ default type concepts ]----------------------//

		namespace concepts {

			template<class T>
			concept json_default_type = registerDefaultValue<std::remove_cvref_t<T>>::value != INVALID;

			template<class T, type EnumType>
			concept json_default_of_type = registerDefaultValue<std::remove_cvref_t<T>>::value == EnumType;

			template<class T>
			concept json_default_boolean = json_default_of_type<T, BOOLEAN>;

			template<class T>
			concept json_default_floating = json_default_of_type<T, NUMBER>;

			template<class T>
			concept json_default_string = json_default_of_type<T, STRING>;

			template<class T>
			concept json_default_array = json_default_of_type<T, ARRAY>;

			template<class T>
			concept json_default_object = json_default_of_type<T, OBJECT>;

			template<class T>
			concept json_default_variant = json_default_of_type<T, VARIANT>;

			template<class T>
			concept json_default_adapter = json_default_of_type<T, ADAPTER>;

			template<class T>
			concept json_default_primitive = (
				json_default_boolean<T> || json_default_floating<T> || json_default_string<T>
			);

			template<class T>
			concept json_default_compounds = (
				json_default_array<T> || json_default_object<T> || json_default_variant<T> || json_default_adapter<T>
			);
		}

		namespace detail {
			template<typename LiteralType, typename Type>
				requires(std::is_nothrow_convertible_v<LiteralType, Type>)
			struct primitive_default_t {
				using type = Type;

				LiteralType value;

				[[nodiscard]] inline type operator()() const {
					return static_cast<type>(value);
				}
			};
		}

		//----------------------[ Primitives ]----------------------//

		struct boolean : detail::primitive_default_t<bool, boolean_t> {};
		template<> struct registerDefaultValue<boolean> : public type_container<BOOLEAN> {};

		struct number : detail::primitive_default_t<double, number_t> {};
		template<> struct registerDefaultValue<number> : public type_container<NUMBER> {};


		//----------------------[ String ]----------------------//

		template<size_t Length>
		struct string {
			using type = std::string;

			char value[Length]{};

			constexpr string(const char (&str)[Length]) {
				std::copy_n(str, Length, value);
			}

			constexpr string(const string_literal<Length> &str) {
				std::copy(str.begin(), str.end(), value);
			}

			constexpr auto size() const {
				return Length - 1;
			}

			[[nodiscard]] inline type operator()() const {
				return { value };
			}
		};

		template<size_t Length>
		struct registerDefaultValue<string<Length>> : public type_container<STRING> {};


		//----------------------[ Array ]----------------------//

		template<concepts::json_default_type auto DefaultElement, concepts::json_default_type auto... DefaultElements>
			requires (
				std::same_as<
					typename decltype(DefaultElement)::type,
					typename decltype(DefaultElements)::type
				> && ...
			)
		struct array {
			static constexpr auto defaultElement = DefaultElement;

			using element_type = decltype(defaultElement)::type;
			using type = std::vector<element_type>;

			[[nodiscard]] inline type operator()() const {
				return { DefaultElement(), DefaultElements()... };
			}
		};

		template<concepts::json_default_type auto ElementType, auto... DefaultValues>
		struct registerDefaultValue<array<ElementType, DefaultValues...>> : public type_container<ARRAY> {};


		//----------------------[ Object ]----------------------//

		template<string_literal Key, concepts::json_default_type auto Value>
		struct named_value {
			static constexpr auto key = Key;
			static constexpr auto value = Value;
			using valueType = decltype(Value);
		};

		template<named_value... Entries>
		struct object {
			using type = object_t<
				named_type<
					decltype(Entries)::key,
					typename decltype(Entries)::valueType::type
				>{}...
			>;

			template<size_t Index>
			[[nodiscard]] inline consteval auto get() const {
				auto findElement = []<size_t StepsLeft, named_value First, named_value...Rest>(auto&& self) {
					if constexpr (StepsLeft == 0) {
						return First.value;
					} else if constexpr (StepsLeft > 0 && sizeof...(Rest) > 0) {
						return self.template operator()<StepsLeft - 1, Rest...>(self);
					} else {
						decltype(First)::__index_out_of_range;
					}
				};
				return findElement.template operator()<Index, Entries...>(findElement);
			}

			template<string_literal Name>
			[[nodiscard]] inline consteval auto get() const {
				constexpr auto index_opt = type::indexOf(Name);
				static_assert(index_opt, "Unknown key");
				return get<*index_opt>();
			}

			[[nodiscard]] inline type operator()() const {
				return type(decltype(Entries)::value()...);
			}
		};

		template<named_value... Entries>
		struct registerDefaultValue<object<Entries...>> : public type_container<OBJECT> {};


		//----------------------[ Variant ]----------------------//

		template<string_literal Key, named_value... Entries>
		struct named_alternative {
			static constexpr auto key = Key;
			static constexpr auto alternative = object<Entries...>{};
			using alternativeType = decltype(alternative);
		};

		template<string DefaultKey, named_alternative... Alternatives>
			requires ( (string_literal(DefaultKey.value) == decltype(Alternatives)::key) || ... )
		struct variant {
			using type = varaint_t<
				named_type<
					decltype(Alternatives)::key,
					typename decltype(Alternatives)::alternativeType::type
				>{}...
			>;

			// Most interfaces were designed for 'string_literal', so this copy is needed
			static constexpr auto defaultLiteral = string_literal(DefaultKey.value);

			template<size_t Index>
			[[nodiscard]] inline static consteval auto get() {
				auto findElement = []<size_t StepsLeft, named_alternative First, named_alternative...Rest>(auto&& self) {
					if constexpr (StepsLeft == 0) {
						return First.alternative;
					} else if constexpr (StepsLeft > 0 && sizeof...(Rest) > 0) {
						return self.template operator()<StepsLeft - 1, Rest...>(self);
					} else {
						decltype(First)::__unknown_name;
					}
				};
				return findElement.template operator()<Index, Alternatives...>(findElement);
			}

			template<string_literal Name>
			[[nodiscard]] inline static consteval auto get() {
				constexpr auto index_opt = type::indexOf(Name);
				static_assert(index_opt, "Unknown key");
				return get<*index_opt>();
			}

			[[nodiscard]] inline type operator()() const {
				constexpr auto defaultAlternative = get<defaultLiteral>();
				return type::template make<defaultLiteral>( defaultAlternative() );
			}
		};

		template<string DefaultKey, named_alternative... Alternatives>
		struct registerDefaultValue<variant<DefaultKey, Alternatives...>> : public type_container<VARIANT> {};


		//----------------------[ Adapter ]----------------------//

		template<class T, typename X, typename Y>
		concept two_way_converter = requires(const X x, const Y y) {
			{ T::convert(x) } -> std::same_as<std::optional<Y>>;
			{ T::revert(y)  } -> std::same_as<std::optional<X>>;
		};

		template<
			concepts::json_default_type default_t, typename runtime_t,
			two_way_converter<typename default_t::type, runtime_t> converter
		>
		struct adapter {
			using type = runtime_t;
			using json_t = default_t::type;

			default_t defaultValue;

			[[nodiscard]] inline runtime_t operator()() const {
				return *converter::convert(defaultValue());
			}

			[[nodiscard]] inline runtime_t convert(const json_t &jsonValue) const {
				return *converter::convert(jsonValue).or_else([&]() {
					return converter::convert(defaultValue());
				});
			}

			[[nodiscard]] inline json_t revert(const runtime_t &from) const {
				return *converter::revert(from).or_else([&]() {
					return std::optional<json_t>{ defaultValue() };
				});
			}
		};

		template<
			concepts::json_default_type default_t, typename runtime_t,
			two_way_converter<typename default_t::type, runtime_t> converter
		>
		struct registerDefaultValue<adapter<default_t, runtime_t, converter>> : public type_container<ADAPTER> {};


		//----------------------[ Default Adapters ]----------------------//

		namespace detail {
			struct number_unsigned_integer_converter {
				using x_t = number_t;
				using y_t = unsigned long long;

				static std::optional<y_t> convert(const x_t &x) {
					y_t y = static_cast<y_t>(std::round(x));

					const auto delta = static_cast<double>(y) - x;
					constexpr auto epsilon = std::numeric_limits<double>::epsilon();

					if (x >= 0 and std::abs(delta) < epsilon) {
						return y;
					} else {
						return std::nullopt;
					}
				};

				static std::optional<x_t> revert(const y_t &y) {
					return { static_cast<double>(y) };
				};
			};
		}

		using unsigned_integer = adapter<number, unsigned long long, detail::number_unsigned_integer_converter>;

		namespace detail {
			struct number_integer_converter {
				using x_t = number_t;
				using y_t = long long;

				static std::optional<y_t> convert(const x_t &x) {
					y_t y = static_cast<y_t>(std::round(x));

					const auto delta = static_cast<double>(y) - x;
					constexpr auto epsilon = std::numeric_limits<double>::epsilon();

					if (std::abs(delta) < epsilon) {
						return y;
					} else {
						return std::nullopt;
					}
				};

				static std::optional<x_t> revert(const y_t &y) {
					return { static_cast<double>(y) };
				};
			};

			using integer_adapter = adapter<number, long long, number_integer_converter>;
		}

		struct integer : public detail::integer_adapter {
			// I mean this is ridiculous, but I'm wayyyy past that point!
			constexpr integer operator -() {
				return integer{ -defaultValue.value };
			}
		};

		template<>
		struct registerDefaultValue<integer> : public type_container<ADAPTER> {};

		namespace detail {
			template<string_literal... Values>
			struct string_enum_converter {
			private:
				static constexpr auto indexer = string_indexer<sizeof...(Values)>(Values...);
			public:
				using x_t = string_t;
				using y_t = unsigned short;

				static  std::optional<y_t> convert(const x_t &x) {
					const auto index_opt = indexer.indexOf(x);
					if (index_opt) {
						return static_cast<y_t>(*index_opt);
					} else {
						return std::nullopt;
					}
				};

				static std::optional<x_t> revert(const y_t &y) {
					const auto name_opt = indexer.nameOf(y);
					if (name_opt) {
						return x_t{ *name_opt };
					} else {
						return std::nullopt;
					}
				};
			};

			template<size_t StrLen, string_literal... Values>
			using enum_adapter = adapter<string<StrLen>, unsigned short, string_enum_converter<Values...>>;
		}

		template<string DefaultValue, string_literal... Values>
		struct enumeration : detail::enum_adapter<DefaultValue.size() + 1, Values...> {
			constexpr enumeration() : detail::enum_adapter<DefaultValue.size() + 1, Values...>{ DefaultValue }{}
		};

		template<string DefaultValue, string_literal... Values>
		struct registerDefaultValue<enumeration<DefaultValue, Values...>> : public type_container<ADAPTER> {};


		//----------------------[ Literals ]----------------------//

		namespace literals {
			constexpr boolean operator""_B(unsigned long long int value) {
				return { static_cast<bool>(value) };
			}

			static constexpr boolean false_B { false };
			static constexpr boolean true_B { true };

			constexpr number operator""_N(long double value) {
				return { static_cast<double>(value) };
			}

			template<string Str>
			constexpr auto operator""_S() {
				return Str;
			}

			constexpr integer operator""_I(unsigned long long i) {
				return { number{ static_cast<double>(i) } };
			}

			constexpr unsigned_integer operator""_U(unsigned long long u) {
				return { number{ static_cast<double>(u) } };
			}
		}
	}

	namespace default_builder {
		using namespace json::safe::default_values;
		using namespace json::safe::default_values::literals;

		template<string_literal Key, auto Value>
		using set = json::safe::default_values::named_value<Key, Value>;

		template<string_literal Key, named_value... Entries>
		using holds = json::safe::default_values::named_alternative<Key, Entries...>;
	}


	using namespace default_values;
	using namespace default_values::concepts;
	namespace tc = transcoding;
	namespace tokens = tc::tokens;

	using base_json_parser_t = tc::parser<tc::tokens::type,
		tokens::end,
		tokens::left_brace,
		tokens::right_brace,
		tokens::comma,
		tokens::colon,
		tokens::string,
		tokens::number,
		tokens::boolean,
		tokens::left_bracket,
		tokens::right_bracket,
		tokens::null
	>;

	class parser : public base_json_parser_t {
	public:
		inline parser(std::istream &is);

		template<json_default_type auto DefaultType>
		inline decltype(DefaultType)::type parse();

	private:
		template<json_default_primitive auto DefaultPrimitive, tokens::type Type>
		inline decltype(auto) parsePrimitive();

		template<json_default_array auto DefaultArray>
		inline decltype(DefaultArray)::type parseArray();

		template<json_default_variant auto DefaultVariant>
		inline decltype(DefaultVariant)::type parseVariant();

		template<json_default_object auto DefaultObject>
		inline decltype(DefaultObject)::type parseObject();

		template<json_default_type auto DefaultValue, type Type>
		inline decltype(DefaultValue)::type parseValue();

		template<json_default_object auto DefaultObject, size_t Size>
		inline void parseEntry(std::bitset<Size> &isSet, typename decltype(DefaultObject)::type &object);

		template<json_default_object auto DefaultObject>
		inline void parseEntries(typename decltype(DefaultObject)::type &object, bool inside = false);

		inline void skipPrimitive();
		inline void skipArray();
		inline void skipObject(bool inside = false);
		inline void skipValue();
	};

	parser::parser(std::istream &is)
		: base_json_parser_t(is) {}


	inline void parser::skipPrimitive() {
		accept();
	}

	inline void parser::skipArray() {
		using enum tokens::type;

		accept(LBRACKET);

		size_t openBrackets = 1;
		constexpr auto brackets = std::array{ ']', '[' };
		do {
			const auto nextBracket = tokens.ignore(brackets);
			openBrackets += nextBracket == '[' ? 1 : -1;
		} while(openBrackets);

		accept();
	}

	inline void parser::skipObject(bool inside) {
		using enum tokens::type;

		size_t openBraces = 0;
		if (not inside) {
			accept(LBRACE);
			openBraces++;
		}

		constexpr auto braces = std::array{ '}', '{' };
		do {
			const auto nextBrace = tokens.ignore(braces);
			openBraces += nextBrace == '{' ? 1 : -1;
		} while(openBraces);

		accept();
	}

	inline void parser::skipValue() {
		switch (token.type()) {
			using enum tokens::type;
			case LBRACE:
				return skipObject();
			case LBRACKET:
				return skipArray();
			case STRING:
			case NUMBER:
			case BOOLEAN:
			case UNDEFINED:
				return skipPrimitive();
			default: {

			}
		}
	}


	template<json_default_primitive auto DefaultPrimitive, tokens::type Type>
	inline decltype(auto) parser::parsePrimitive() {

		using default_primitive_t = decltype(DefaultPrimitive);
		using primitive_t = typename default_primitive_t::type;

		primitive_t primitive(token.get<Type>());

		accept();

		return primitive;
	}

	template<json_default_array auto DefaultArray>
	inline decltype(DefaultArray)::type parser::parseArray() {
		using enum tokens::type;

		using default_array_t = decltype(DefaultArray);
		typename default_array_t::type array;

		accept();

		if (token.type() != RBRACKET) {
			array.push_back(parse<default_array_t::defaultElement>());
			while (token.type() != RBRACKET) {
				accept(COMMA);
				array.push_back(parse<default_array_t::defaultElement>());
			}
		}
		accept(RBRACKET);

		return array;
	}


	template<json_default_object auto DefaultObject, size_t Size>
	inline void parser::parseEntry(std::bitset<Size> &isSet, typename decltype(DefaultObject)::type &object) {
		using enum tokens::type;

		using default_object_t = decltype(DefaultObject);
		using object_t = typename default_object_t::type;

		const auto key = accept(STRING).get<STRING>();
		// Lifetime of key ends with next accept so indexOf needs to be done here
		const auto index_opt = object_t::indexOf(key);

		accept(COLON);

		if (index_opt and not isSet[*index_opt]) {
			isSet[*index_opt] = true;
			// inline a switch for the correct member default value
			constexpr_for::index<Size>([&]<size_t Index>() {
				if (Index == *index_opt) {
					constexpr auto defaultMember = DefaultObject.template get<Index>();
					object.template get<Index>() = parse<defaultMember>();
					return true;
				}
				return false;
			});
		} else {
			// key is not specified in default object and can be ignored
			skipValue();
		}
	}

	template<json_default_object auto DefaultObject>
	inline void parser::parseEntries(typename decltype(DefaultObject)::type &object, bool inside) {
		using enum tokens::type;

		using default_object_t = decltype(DefaultObject);
		using object_t = default_object_t::type;

		constexpr auto numEntries = object_t::size();

		// Since entries need to be parsed in the same order as the json file,
		// which might not match the order of entries in the default object,
		// every parsed key needs to be matched to a default entry and marked in 'isSet'
		std::bitset<numEntries> isSet{};

		if (not inside and token.type() != RBRACE) {
			parseEntry<DefaultObject>(isSet, object);
		}

		while (token.type() != RBRACE) {
			accept(COMMA);
			parseEntry<DefaultObject>(isSet, object);
		}

		// All entries that were not provided by the json file need to be filled in with default values
		constexpr_for::index<numEntries>([&]<size_t Index>() {
			if (not isSet[Index]) {
				constexpr auto defaultMember = DefaultObject.template get<Index>();
				object.template get<Index>() = defaultMember();
				// isSet[Index] = true;
			}
			return false;
		});

	}

	template<json_default_object auto DefaultObject>
	inline decltype(DefaultObject)::type parser::parseObject() {
		using enum tokens::type;

		accept();

		using default_object_t = decltype(DefaultObject);
		using object_t = typename default_object_t::type;
		object_t object;

		parseEntries<DefaultObject>(object);
		accept(RBRACE);

		return object;
	}

	template<json_default_variant auto DefaultVariant>
	inline decltype(DefaultVariant)::type parser::parseVariant() {
		using enum tokens::type;

		using default_variant_t = decltype(DefaultVariant);
		using variant_t = typename default_variant_t::type;

		accept();

		// variant needs to begin with "_type": "ENUN_VALUE"
		size_t typeIndex = SIZE_MAX;
		if (token.type() == STRING) {

			auto typeKey = token.get<STRING>();
			const auto typeKeyCorrect = typeKey == "_type";
			accept();

			accept(COLON);

			if (typeKeyCorrect and token.type() == STRING) {
				const auto typeValue = token.get<STRING>();
				typeIndex = variant_t::indexOf(typeValue).value_or(SIZE_MAX);
				accept();
			}
		}

		if (typeIndex == SIZE_MAX) {
			skipObject(true);
			return DefaultVariant();
		}

		variant_t variant(in_place_dynamic_index{ typeIndex });
		constexpr auto numAlternatives = variant_t::size();

		// inline a switch statement for every possible alternative
		constexpr_for::index<numAlternatives>([&]<size_t Index>() {
			if (Index == typeIndex) {
				constexpr auto defaultObject = default_variant_t::template get<Index>();
				// all members need to be initialized
				parseEntries<defaultObject>(variant.template get<Index>(), true);
				return true;
			}
			return false;
		});

		accept(RBRACE);

		return variant;
	}


	template<json_default_type auto DefaultType, type Type>
	inline decltype(DefaultType)::type parser::parseValue() {
		if constexpr (Type == BOOLEAN) {
			return parsePrimitive<DefaultType, tokens::type::BOOLEAN>();
		} else if constexpr (Type == NUMBER) {
			return parsePrimitive<DefaultType, tokens::type::NUMBER>();
		} else if constexpr (Type == STRING) {
			return parsePrimitive<DefaultType, tokens::type::STRING>();
		} else if constexpr (Type == ARRAY) {
			return parseArray<DefaultType>();
		} else if constexpr (Type == OBJECT) {
			return parseObject<DefaultType>();
		} else if constexpr (Type == VARIANT) {
			return parseVariant<DefaultType>();
		} else {
			decltype(DefaultType)::_error_type;
		}
	}

	template<json_default_type auto BaseDefaultType>
	inline decltype(BaseDefaultType)::type parser::parse() {

		constexpr auto baseType = typeOfDefault(BaseDefaultType);

		constexpr auto defaultType = []() {
			if constexpr (baseType == ADAPTER) {
				return BaseDefaultType.defaultValue;
			} else {
				return BaseDefaultType;
			}
		}();

		constexpr auto type = typeOfDefault(defaultType);
		using default_t = decltype(defaultType);
		using value_t = default_t::type;

		value_t value{};

		auto foundType = false;
		using json_type = json::safe::type;
		using token_type = tokens::type;
		constexpr_for::values<
			std::pair{ json_type::BOOLEAN	, token_type::BOOLEAN	},
			std::pair{ json_type::NUMBER	, token_type::NUMBER	},
			std::pair{ json_type::STRING	, token_type::STRING	},
			std::pair{ json_type::ARRAY		, token_type::LBRACKET	},
			std::pair{ json_type::OBJECT	, token_type::LBRACE	},
			std::pair{ json_type::VARIANT	, token_type::LBRACE	}
		>([&]<auto p>() {
			if constexpr (p.first == type) {
				if (token.type() == p.second) {
					value = parseValue<defaultType, p.first>();
					foundType = true;
				}
				return true;
			}
			return false;
		});

		if (not foundType) {
			skipValue();
			value = defaultType();
		}

		if constexpr (baseType == ADAPTER) {
			return BaseDefaultType.convert(value);
		} else {
			return value;
		}
	}

	using base_json_serializer_t = tc::serializer<tokens::type,
		tokens::end,
		tokens::left_brace,
		tokens::right_brace,
		tokens::comma,
		tokens::colon,
		tokens::string,
		tokens::number,
		tokens::boolean,
		tokens::left_bracket,
		tokens::right_bracket,
		tokens::null
	>;

	class serializer : public base_json_serializer_t {
	public:
		using base_json_serializer_t::serializer;

		template<json_default_type auto DefaultValue>
		inline void serialize(const decltype(DefaultValue)::type& value);

	private:
		template<json_default_array auto DefaultArray>
		inline void serializeArray(const decltype(DefaultArray)::type& value);

		template<json_default_object auto DefaultObject>
		inline void serializeObject(const decltype(DefaultObject)::type& value);

		template<json_default_variant auto DefaultVariant>
		inline void serializeVariant(const decltype(DefaultVariant)::type& value);

		template<json_default_object auto DefaultObject>
		inline void serializeEntries(const decltype(DefaultObject)::type &object, bool inside = false);
	};

	template<json_default_array auto DefaultArray>
	inline void serializer::serializeArray(const decltype(DefaultArray)::type& value) {
		using DefaultArray_t = decltype(DefaultArray);
		using enum tc::detokenizer_instructions;

		tokens << lbracket;
		if (value.empty()) {
			tokens << space;
		} else {
			tokens << indent << newline;
			auto it = value.begin();
			while (it != value.end()) {
				serialize<DefaultArray_t::defaultElement>(*it);
				if (++it != value.end()) {
					tokens << comma << newline;
				}
			}
			tokens << outdent;
		}
		tokens << newline << rbracket;
	}

	template<json_default_object auto DefaultObject>
	inline void serializer::serializeObject(const decltype(DefaultObject)::type& value) {
		using enum tc::detokenizer_instructions;

		tokens << lbrace << indent << newline;
		serializeEntries<DefaultObject>(value);
		tokens << outdent << newline << rbrace;
	}


	template<json_default_variant auto DefaultVariant>
	inline void serializer::serializeVariant(const decltype(DefaultVariant)::type& value) {
		using DefaultVariant_t = decltype(DefaultVariant);
		using enum tc::detokenizer_instructions;

		tokens << lbrace << indent << newline;

		value.visit([&]<typename Entry>(const Entry &entry) {
			tokens << string("_type") << colon << space << string(Entry::key.c_str());
			constexpr auto defaultAlternative = DefaultVariant_t::template get<Entry::index>();
			serializeEntries<defaultAlternative>(entry.value, true);
		});

		tokens << outdent << newline << rbrace;
	}

	template<json_default_object auto DefaultObject>
	inline void serializer::serializeEntries(const decltype(DefaultObject)::type &object, bool inside) {
		using DefaultObject_t = decltype(DefaultObject);
		using enum tc::detokenizer_instructions;

		constexpr auto size = DefaultObject_t::type::size();
		if constexpr (size > 0) {
			if (inside) {
				tokens << comma << newline;
			}

			const auto lastIndex = size - 1;

			object.apply([&](const auto &... entries) {
				constexpr_for::indexed_arguments([&]<size_t Index, typename Entry>(const Entry &entry) {
					tokens << string(Entry::key.c_str()) << colon << space;

					constexpr auto defaultMember = DefaultObject.template get<Index>();
					serialize<defaultMember>(entry.value);

					if constexpr (Index != lastIndex)
						tokens << comma << newline;

					return false;
				}, entries...);
			});
		}
	}

	template<json_default_type auto DefaultValue>
	inline void serializer::serialize(const decltype(DefaultValue)::type& value) {
		constexpr auto type = default_values::typeOfDefault(DefaultValue);

		using jsv = json::safe::default_values::type;
		using tkn = tokens::type;
		using jtkn = tc::json_token_t;

		if constexpr (type == jsv::BOOLEAN) {
			tokens << jtkn::make<tkn::BOOLEAN>(value);
		} else if constexpr (type == jsv::NUMBER) {
			tokens << jtkn::make<tkn::NUMBER>(value);
		} else if constexpr (type == jsv::STRING) {
			tokens << jtkn::make<tkn::STRING>(value);
		} else if constexpr (type == jsv::ARRAY) {
			serializeArray<DefaultValue>(value);
		} else if constexpr (type == jsv::ADAPTER) {
			serialize<DefaultValue.defaultValue>(DefaultValue.revert(value));
		} else if constexpr (type == jsv::OBJECT) {
			serializeObject<DefaultValue>(value);
		} else if constexpr (type == jsv::VARIANT) {
			serializeVariant<DefaultValue>(value);
		} else {
			DefaultValue.__unknown_type;
		}
	}
}
