#pragma once
#include <type_traits>


namespace con
{
	template<typename T, typename U>
	concept Same = requires()
	{
		std::is_same_v<T, U>;
		std::is_same_v<U, T>;
	};

	template<typename T>
	concept Container = requires(T a)
	{
		Same<decltype(a.size()), size_t>;
		std::is_pointer_v<decltype(a.data())>;
		a.begin();
		a.end();
		Same<std::remove_pointer<decltype(a.data())>, typename T::value_type>;
		Same<decltype(a.begin()), typename T::iterator>;
		Same<decltype(a.end()), typename T::iterator>;
	};

	template<typename T>
	concept Blittable = requires()
	{
		std::is_trivial_v<T>;
		std::is_standard_layout_v<T>;
	};

	template<typename T, typename... Args>
	concept InvocableWith = requires()
	{
		std::is_invocable_v<T, Args...>;
	};

	template<typename T>
	concept Arithmetic = requires(T a, T b)
	{
		Same<decltype(a + b), T>;
		Same<decltype(a - b), T>;
		Same<decltype(a / b), T>;
		Same<decltype(a *b), T>;
		Same<std::remove_reference_t<decltype(a += b)>, T>;
		Same<std::remove_reference_t<decltype(a -= b)>, T>;
		Same<std::remove_reference_t<decltype(a /= b)>, T>;
		Same<std::remove_reference_t<decltype(a *= b)>, T>;
	};
}