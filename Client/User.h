#pragma once
/*
 * Basic system type definitions, taken from the BSD file sys/types.h.
 */
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;


namespace Andrew
{
	struct User
	{
		bool auth = 0;
		u_long id = 0;
	};
}