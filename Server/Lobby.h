#pragma once
#include "Types.h"
#include "User.h"
#include <string>
#include "ByteBuffer.h"
#include "Registrator.h"

namespace Andrew
{
	class Lobby
	{
	public:
		Lobby() = default;
		Lobby(u_long id, u_long ownerId, u_long dataSize, u_long chunks, const std::string& dataName)
			: id(id), ownerId(ownerId), buffer(dataSize), dataName(dataName), chunks(chunks)
		{
		}
	public:
		u_long id;
		u_long ownerId;
		u_long chunks;
		ByteBuffer buffer;
		std::string dataName;
		double lifetime = 0.0f;
		bool uploaded = 0;
	};
}