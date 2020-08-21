#pragma once


template<typename T, class ID>
class StrongTypedef
{
public:

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

private:
	T value;
};