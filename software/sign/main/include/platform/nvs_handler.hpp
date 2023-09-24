#pragma once

// I'm sorry, I swear this seemed like a good idea an hour ago...

#include "nvs_flash.h"
#include <esp_log.h>

#include <type_traits>
#include <concepts>
#include <array>
#include <algorithm>
#include <util/string_literal.hpp>
#include <util/function.hpp>
#include <util/uix.hpp>

namespace detail {
	template<typename T>
	concept enum_t = std::is_enum_v<T> && std::is_unsigned_v<std::underlying_type_t<T>>;

	template<typename T>
	concept default_constructable = std::is_default_constructible_v<T>;

	template<enum_t auto EnumValue, typename T, auto GetValue>
		requires (ztu::supplier<decltype(GetValue), T>)
	struct type_descriptor_t {
		static constexpr auto enumValue = EnumValue;
		using type = T;
		inline static const T getValue() {
			return GetValue();
		} 
	};
}

template<detail::enum_t auto Key, class TypeDesc>
struct nvs_entry {
	using keyType = decltype(Key);
	static constexpr auto key = Key;
	using typeDesc = TypeDesc;
};

namespace detail {
	template<size_t I, class T>
	struct indexed : public T {
		static constexpr auto index = I;
	};
	
	template<enum_t auto EnumValue, size_t Index, nvs_entry... Entries>
	struct find_nvs_entry {
		using type = void;
	};

	template<enum_t auto EnumValue, size_t Index, nvs_entry First, nvs_entry... Rest>
	struct find_nvs_entry<EnumValue, Index, First, Rest...> : public std::conditional<
		EnumValue == decltype(First)::key,
		indexed<Index, decltype(First)>,
		typename find_nvs_entry<EnumValue, Index + 1, Rest...>::type
	> {};

	template<enum_t auto EnumValue, nvs_entry... Entries>
	using find_nvs_entry_t = typename find_nvs_entry<EnumValue, 0, Entries...>::type;


	template<std::unsigned_integral... Is>
	constexpr bool is_unique_sequence(const Is... integers) {
		std::array<size_t, sizeof...(Is)> data{ integers... };
		std::sort(data.begin(), data.end());
		return std::adjacent_find(data.begin(), data.end()) == data.end();
	}
	
	constexpr auto numDigits(std::unsigned_integral auto n, std::unsigned_integral auto base = 10U) {
		if (n < base) return 1U;
		auto digits = 0U;
		while (n > 0) {
			n /= base;
			digits++;
		}
		return digits;
	}

	template<std::unsigned_integral auto N, std::unsigned_integral auto B>
	constexpr auto intToString() {
		constexpr auto length = numDigits(N, B);
		std::array<char, length + 1> str;
		auto it = str.rbegin();
		*it++ = '\0';
		auto num = N;
		do {
			*it++ = (char) ('0' + num % B);
			num /= B;
		} while (it != str.rend());
		return str;
	}

	template<std::unsigned_integral auto... Is>
	constexpr auto createUniqueKeys(std::index_sequence<Is...>) {
		constexpr auto base = unsigned('~') - unsigned('0') + 1U;
		return std::tuple( intToString<Is, base>()... );
	}
};

template<nvs_entry... Entries>
	requires (
		detail::is_unique_sequence(
			static_cast<
				std::underlying_type_t<typename decltype(Entries)::keyType>
			>(Entries.key)...
		)
	)
class nvs_handler {

private:
	template<detail::enum_t auto EnumValue>
	using getEntry = typename detail::find_nvs_entry_t<EnumValue, Entries...>;

	template<detail::enum_t auto EnumValue>
	using getType = typename getEntry<EnumValue>::typeDesc::type;


	static bool initNVS() {
		static bool initialized = false;

		if (not initialized) {
			auto ret = nvs_flash_init();
			if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
				if (nvs_flash_erase() != ESP_OK)
					return false;
				ret = nvs_flash_init();
			}
			initialized = ret == ESP_OK;
		}

		ESP_LOGI(tag, "NVS init %s", initialized ? "success" : "failure");

		return initialized;
	}

public:
	nvs_handler() {
		ESP_LOGI(tag, "Creating nvs_handler instance");
	};

	bool open(const char *name) {
		if (not initNVS()) return false;
		return nvs_open(name, NVS_READWRITE, &handle) == ESP_OK;
	}

	template<detail::enum_t auto EnumValue>
	void set(const getType<EnumValue>& value, bool& wroteData = ignoreSuccess) {
		using entry = getEntry<EnumValue>;
		using desc = entry::typeDesc;
		constexpr auto &key = std::get<entry::index>(keys);
		const auto ret = desc::store(handle, key.data(), value);
		wroteData = ret == ESP_OK;
		if (not wroteData) {
			ESP_LOGE(tag, "Error '0x%x' while storing value with key '%s'.", ret, key.data());
		}
	}

	template<detail::enum_t auto EnumValue>
	auto get(bool& readData = ignoreSuccess) {
		using entry = getEntry<EnumValue>;
		using desc = entry::typeDesc;
		using type = desc::type;

		type value;
		constexpr auto &key = std::get<entry::index>(keys);
		const auto ret = desc::retrieve(handle, key.data(), value);
		readData = ret == ESP_OK;
		if (not readData) {
			value = desc::getValue();
			ESP_LOGE(tag, "Error '0x%x' while retrieving value with key '%s'. Returning default value.", ret, key.data());
		}
		return value;
	}

	esp_err_t save() {
		return nvs_commit(handle);
	}

private:
	static constexpr auto tag = "NVS_HANDLER";
	inline static bool ignoreSuccess = false;
	static constexpr auto keys = detail::createUniqueKeys(
		std::make_index_sequence<sizeof...(Entries)>()
	);

	nvs_handle_t handle{ };	
};

namespace default_types {

	enum class default_type_enum : uint8_t {
		U8, I8,
		U16, I16,
		U32, I32,
		U64, I64,
		STRING,
		ARRAY,
		OBJECT
	};
	using enum default_type_enum;

	template<typename T>
	using nvs_getter = esp_err_t(*)(nvs_handle_t, const char*, T*);

	template<typename T>
	using nvs_setter = esp_err_t(*)(nvs_handle_t, const char*, T);

	template<
	    default_type_enum EnumValue,
		typename T,
		auto GetValue,
		nvs_getter<T> Getter,
		nvs_setter<T> Setter
	>
	struct basic_default_descriptor : public detail::type_descriptor_t<EnumValue, T, GetValue> {
		static esp_err_t retrieve(nvs_handle_t handle, const char *key, T &dst) {
			return Getter(handle, key, &dst);
		}
		static esp_err_t store(nvs_handle_t handle, const char *key, const T &src) {
			return Setter(handle, key, src);
		}
	};
	
	template<auto GetValue>
	using u8_t = basic_default_descriptor<U8, u8, GetValue, nvs_get_u8, nvs_set_u8>;
	template<auto GetValue>
	using i8_t = basic_default_descriptor<I8, i8, GetValue, nvs_get_i8, nvs_set_i8>;

	template<auto GetValue>
	using u16_t = basic_default_descriptor<U16, u16, GetValue, nvs_get_u16, nvs_set_u16>;
	template<auto GetValue>
	using i16_t = basic_default_descriptor<I16, i16, GetValue, nvs_get_i16, nvs_set_i16>;

	template<auto GetValue>
	using u32_t = basic_default_descriptor<U32, u32, GetValue, nvs_get_u32, nvs_set_u32>;
	template<auto GetValue>
	using i32_t = basic_default_descriptor<I32, i32, GetValue, nvs_get_i32, nvs_set_i32>;

	template<auto GetValue>
	using u64_t = basic_default_descriptor<U64, u64, GetValue, nvs_get_u64, nvs_set_u64>;
	template<auto GetValue>
	using i64_t = basic_default_descriptor<I64, i64, GetValue, nvs_get_i64, nvs_set_i64>;

	template<size_t MaxSize, auto GetValue>
	struct string_t : public detail::type_descriptor_t<STRING, ztu::string_literal<MaxSize + 1>, GetValue> {
		using type = ztu::string_literal<MaxSize + 1>;
		static esp_err_t retrieve(nvs_handle_t handle, const char *key, type &dst) {
			esp_err_t ret;

			size_t size;
			if ((ret = nvs_get_str(handle, key, nullptr, &size)) != ESP_OK)
				return ret;

			if (size > type::max_size)
				return ESP_ERR_NVS_VALUE_TOO_LONG;

			dst.resize(size);
			if ((ret = nvs_get_str(handle, key, dst.begin(), &size)) != ESP_OK)
				return ret;

			return ESP_OK;
		}

		static esp_err_t store(nvs_handle_t handle, const char *key, const type &src) {
			return nvs_set_str(handle, key, src.c_str()); 
		}
	};

	template<typename T, size_t MaxElements, auto GetValue>
	struct array_t : public detail::type_descriptor_t<ARRAY, std::array<T, MaxElements>, GetValue>  {
		using type = std::array<T, MaxElements>;
		static constexpr auto numBytes = MaxElements * sizeof(T);
		static esp_err_t retrieve(nvs_handle_t handle, const char *key, type &dst) {
			esp_err_t ret;

			size_t size;
			if ((ret = nvs_get_blob(handle, key, nullptr, &size)) != ESP_OK)
				return ret;

			if (size > dst.size())
				return ESP_ERR_NVS_VALUE_TOO_LONG;
			
			if ((ret = nvs_get_blob(handle, key, reinterpret_cast<char*>(dst.data()), &size)) != ESP_OK)
				return ret;

			return ESP_OK; 
		}

		static esp_err_t store(nvs_handle_t handle, const char *key, const type &src) {
			return nvs_set_blob(handle, key, reinterpret_cast<const char*>(src.data()), numBytes);
		}
	};

	template<class T, auto GetValue>
	struct object_t : public detail::type_descriptor_t<OBJECT, T, GetValue> {
		using type = T;
		static esp_err_t retrieve(nvs_handle_t handle, const char *key, type &dst) {
			esp_err_t ret;

			size_t size;
			if ((ret = nvs_get_blob(handle, key, nullptr, &size)) != ESP_OK)
				return ret;

			if (size != sizeof(T))
				return ESP_ERR_NVS_VALUE_TOO_LONG;
			
			if ((ret = nvs_get_blob(handle, key,  &dst, &size)) != ESP_OK)
				return ret;

			return ESP_OK; 
		}

		static esp_err_t store(nvs_handle_t handle, const char *key, const type &src) {
			return nvs_set_blob(handle, key, reinterpret_cast<const char*>(&src), sizeof(T));
		}
	};
}
