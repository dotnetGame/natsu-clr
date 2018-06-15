#pragma once

#define THROW_ALWAYS(exception,...) throw exception(__VA_ARGS__);
#define THROW_IF_NOT(expr,exception,...) {if(!(expr)) throw exception(__VA_ARGS__);}

template<class T>
T align(T value, size_t base)
{
	auto r = value % base;
	return T(r ? value + (base - r) : value);
}
