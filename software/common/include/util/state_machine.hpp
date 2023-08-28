#pragma once

#include <tuple>
#include <array>
#include <variant>
#include <span>


#include "pack.hpp"
#include "function.hpp"

template<class F, class... States>
class state_machine {
private:
	template<class State>
	using run_t = decltype(&State::run);

	using variant_t = std::variant<
		std::monostate,
		typename ztu::pack_of<ztu::function::args_t<run_t<States>>>
			::template erase<0>
			::template transform<std::remove_cvref>
			::template apply<std::tuple>...
	>;
	using enum_t = ztu::function::ret_t<run_t<ztu::first<States...>>>;

public:
	inline explicit state_machine(F&& on_state_change);

	template<class State, typename... Args>
	inline enum_t run(enum_t InitialState, Args&&... args);

	template<typename... Args>
	inline enum_t run(enum_t initial_state, Args&&... args);

private:
	static constexpr auto invalid_index = 0;
	inline static constexpr std::size_t to_index(enum_t e);

private:

	variant_t m_state{};
	F m_on_state_change; // bool (*)(enum_t, enum_t&)
};

#define INCLUDE_STATE_MACHINE_IMPLEMENTATION
#include <util/state_machine.ipp>
#undef INCLUDE_STATE_MACHINE_IMPLEMENTATION
