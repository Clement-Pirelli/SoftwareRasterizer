#pragma once

template<typename T, class ID>
class StrongTypedef
{
public:

	StrongTypedef() : value()
	{
	}

	StrongTypedef(const T& t)
	{
		value = t;
	}
	
	StrongTypedef(T&& t)
	{
		value = std::forward<T>(t);
	}

	StrongTypedef<T, ID> &operator=(const T& t)
	{
		value = t;
		return *this;
	}

	StrongTypedef<T, ID> &operator=(const T&& t)
	{
		value = std::forward<T>(t);
		return *this;
	}

	T & get()
	{
		return value;
	}

	const T &get() const
	{
		return value;
	}

	explicit operator T() const
	{
		return value;
	}

	bool operator==(const StrongTypedef<T, ID> &other) const 
	{
		return other.value == value;
	}

private:
	T value;
};

namespace std
{
	template<typename T, class ID>
	struct hash<StrongTypedef<T, ID>> {
		//member types
		using argument_type = StrongTypedef<T, ID>;
		using result_type = std::size_t;

		result_type operator()(argument_type const &key) const {
			return std::hash<T>()(key.get());
		}
	};
}