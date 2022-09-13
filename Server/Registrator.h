#pragma once
#include <unordered_map>
/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

namespace Andrew
{
	template<class T>
	class Registrator
	{
	public:
		Registrator() = default;
		std::unordered_map<u_long, T> data;
		u_long id = 0;
	};
}