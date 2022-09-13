#pragma once
#include "Types.h"

namespace Andrew
{
	struct User
	{
		User() = default;
		User(u_long id) : id(id) {}
		u_long id;
	};
}