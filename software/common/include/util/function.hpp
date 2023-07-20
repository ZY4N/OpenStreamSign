#pragma once

#include <tuple>
#include <concepts>
#include <functional>

namespace ztu {
	template<typename T, typename F>
	concept function = requires(T t) {
		{ std::function<std::remove_pointer_t<F>>(t) };
	};

	template<typename T>
	concept runnable = function<T, void()>;

	template<typename T, typename R>
	concept supplier = function<T, R()>;

	template<typename T, typename... Ps>
	concept consumer = function<T, void(Ps...)>;

	template<typename T, typename... Ps>
	concept predicate = function<T, bool(Ps...)>;


	template<typename>
	struct meta;

	template<typename R, typename... Ps>
	struct meta<R(*)(Ps...)> {
		using return_type = R;
		using parameter_type = std::tuple<Ps...>;
		static constexpr bool is_const = false;
	};

	template<class C, typename R, typename... Ps>
	struct meta<R(C::*)(Ps...)> {
		using class_type = C;
		using return_type = R;
		using parameter_type = std::tuple<Ps...>;
		static constexpr bool is_const = false;
	};

	template<class C, typename R, typename... Ps>
	struct meta<R(C::*)(Ps...) const> {
		using class_type = C;
		using return_type = R;
		using parameter_type = std::tuple<Ps...>;
		static constexpr bool is_const = true;
	};

	template<typename F>
	using class_t = typename meta<F>::class_type;

	template<typename F>
	using return_t = typename meta<F>::return_type;

	template<typename F>
	using param_t = typename meta<F>::parameter_type;

	template<typename F>
	constexpr bool is_const_v = meta<F>::is_const;
}
