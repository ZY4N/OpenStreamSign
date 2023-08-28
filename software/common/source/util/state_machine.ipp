#ifndef INCLUDE_STATE_MACHINE_IMPLEMENTATION
#error "Never include this file directly. Instead include state_machine.hpp"
#endif

#include <algorithm>
//#include <cassert>
#include <util/variant_visit.hpp>


namespace detail {


	template<typename T>
	struct same_as {
		template<typename U>
		using cmp = std::is_same<T, U>;
	};
	

	template<class>
	struct tuple_from {
		using type = std::tuple<>;
	};

	template<typename... Ts>
	struct tuple_from<std::tuple<Ts...>> {
		using type = std::tuple<Ts...>;
	};


	template<std::size_t SrcIndex, std::size_t DstIndex, typename T, typename... Args>
	inline void variant_tuple_move_impl(T& var, Args&&... args) {
		using var_t = std::remove_cvref_t<T>;

		using src_t = std::variant_alternative_t<SrcIndex, var_t>;
		using dst_t = std::variant_alternative_t<DstIndex, var_t>;

		using src_tuple_t = tuple_from<src_t>::type;
		using dst_tuple_t = tuple_from<dst_t>::type;

		constexpr auto src_tpl_size = std::tuple_size_v<src_tuple_t>;
		constexpr auto dst_tpl_size = std::tuple_size_v<dst_tuple_t>;
		constexpr auto max_match_size = std::min(src_tpl_size, dst_tpl_size);

		constexpr auto num_member_matches = [&]() {
			std::size_t count = 0;
			ztu::for_each::index<max_match_size>([&]<auto Index>() {
				if constexpr (std::is_same_v<
					std::tuple_element_t<Index, src_tuple_t>,
					std::tuple_element_t<Index, dst_tuple_t>
				>) {
					count++;
					return false;
				}
				return true;
			});
			return count;
		}();

		constexpr auto default_members_offset = num_member_matches + sizeof...(Args);
		static_assert(default_members_offset <= dst_tpl_size);
		constexpr auto num_default_members = dst_tpl_size - default_members_offset;

		[&]<auto... Is, auto... Js>(std::index_sequence<Is...>, std::index_sequence<Js...>) {
			auto &&src_alternative = *std::get_if<SrcIndex>(&var); // remove redundant index check
			/*if constexpr (std::is_constructible_v<dst_tuple_t,
				std::add_lvalue_reference_t<std::tuple_element_t<Is, dst_tuple_t>>...,
				std::add_lvalue_reference_t<Args>...,
				std::add_lvalue_reference_t<std::tuple_element_t<default_members_offset + Js, dst_tuple_t>>...
			>) {*/
				var.template emplace<DstIndex>(
					std::tuple_element_t<Is, src_tuple_t>{
						std::move(std::get<Is>(src_alternative))
					}...,
					std::forward<Args>(args)...,
					std::tuple_element_t<default_members_offset + Js, dst_tuple_t>{}...
				);
			/*} else {
				// Certain transitions may not compile and that's ok
				// as long as these transitions are not used at runtime.
				//assert(false); // TODO find solution
				vTaskDelete(nullptr);
			}*/
			return false;
		}(std::make_index_sequence<num_member_matches>{}, std::make_index_sequence<num_default_members>{});
	}

	template<class Variant, typename... Args>
	inline void variant_tuple_move(Variant &variant, std::size_t dst_index, Args&&... args) {
		// This function is responsible for a huge explosion in binary size...s
		const auto src_index = variant.index();
		constexpr auto size = std::variant_size_v<Variant>;
		ztu::for_each::index<size>([&]<auto I>() {
			return ztu::for_each::index<size>([&]<auto J>() {
				if (I == src_index and J == dst_index) {
					if constexpr (I != J) {
						variant_tuple_move_impl<I, J>(variant, std::forward<Args>(args)...);
					}
					return true;
				}
				return false;
			});
		});
	}
}

template<class F, class... States>
constexpr std::size_t state_machine<F, States...>::to_index(enum_t e) {
	ssize_t index = invalid_index;
	ztu::for_each::indexed_type<States...>([&]<auto Index, typename State>() {
		static constexpr auto &states = State::states;
		if (std::find(states.begin(), states.end(), e) != states.end()) {
			index = Index + 1;
			return true;
		}
		return false;
	});
	return index;
}

template<class F, class... States>
state_machine<F, States...>::state_machine(F&& on_state_change)
	: m_on_state_change{ std::forward<F>(on_state_change) }{}


template<class F, class... States>
template<class State, typename... Args>
state_machine<F, States...>::enum_t state_machine<F, States...>::run(
	typename state_machine<F, States...>::enum_t initial_state,
	Args&&... args
) {

	ztu::for_each::indexed_type<States...>([&]<auto Index, typename T>() {
		if constexpr (std::same_as<T, State>) {
			m_state.template emplace<Index>(std::forward<Args>(args)...);
			return true;
		}
		return false;
	});

	enum_t curr_state, next_state{ initial_state };
	do {
		const auto next_index = to_index(next_state);

		detail::variant_tuple_move(
			m_state, next_index
		);

		curr_state = next_state;

		ztu::indexed_visit([&]<auto Index>(auto& data) {
			if constexpr (Index != invalid_index) {
				std::apply([&](auto&... args) {
					using state_t = ztu::at<Index - 1, States...>;
					next_state = state_t::run(curr_state, args...);
				}, data);
			} else {
				assert(false);
			}
		}, m_state);
		
	} while (m_on_state_change(curr_state, next_state));

	return next_state;
}

template<class F, class... States>
template<typename... Args>
state_machine<F, States...>::enum_t state_machine<F, States...>::run(state_machine<F, States...>::enum_t initial_state, Args&&... args) {
	const auto initial_index = to_index(initial_state);
	enum_t state = initial_state;
	[&]<auto... Is>(std::index_sequence<Is...>) {
		[[maybe_unused]] const auto ran = (
			[&]<auto I>() {
				using state_t = std::variant_alternative_t<I, variant_t>;
				if constexpr (
					not std::same_as<state_t, std::monostate> and
					std::is_constructible_v<state_t, Args...>
				) {
					if (I == initial_index) {
						state = run<state_t, Args...>(initial_state, std::forward<Args>(args)...);
						return true;
					}
				}
				return false;
			}.template operator()<Is>() or ...
		);
		// assert(ran); // TODO find solution
	}(std::make_index_sequence<std::variant_size_v<variant_t>>{});
	return state;
}
