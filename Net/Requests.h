#pragma once
#include <cstdint>
#include "Package.h"

namespace Andrew
{
	namespace Request
	{
		struct Ping
		{
		};

		struct Auth
		{
		};

		struct LobbyReg
		{
			u_long id = 0;
			u_long dataSize = 0;
			u_long nameSize = 0;
			u_long chunks = 0;
			char dataName[
				PayloadSize
					- sizeof(id)
					- sizeof(dataSize)
					- sizeof(nameSize)
					- sizeof(chunks)
			] = { 0 };
		};

		struct LobbyConnect
		{
			u_long id = 0;
			u_long lobbyId = 0;
		};

		struct SendPicChunk
		{
			u_long id = 0;
			u_long lobbyId = 0;
			u_long chunkId = 0;
			u_long size = 0;
			u_long offset = 0;
			char bin[
				PayloadSize
					- sizeof(id)
					- sizeof(lobbyId)
					- sizeof(chunkId)
					- sizeof(size)
					- sizeof(offset)
			] = { 0 };
		};

		struct GetPicChunk
		{
			u_long lobbyId = 0;
			u_long chunkId = 0;
		};
	}
	
	namespace Response
	{
		struct Ping
		{
			bool status = 0;
		};

		struct Auth
		{
			bool status = 0;
			u_long userId = 0;
		};

		struct LobbyReg
		{
			bool status = 0;
			u_long lobbyId = 0;
		};

		struct LobbyConnect
		{
			bool status = 0;
			u_long dataSize = 0;
			u_long nameSize = 0;
			char dataName[
				PayloadSize
					- sizeof(status)
					- sizeof(dataSize)
			] = { 0 };
		};

		struct SendPicChunk
		{
			bool status = 0;
			u_long chunkId = 0;
		};

		struct GetPicChunk
		{
			bool status = 0;
			u_long chunkId = 0;
			u_long size = 0;
			u_long offset = 0;
			char bin[
				PayloadSize
					- sizeof(status)
					- sizeof(chunkId)
					- sizeof(size)
					- sizeof(offset)
			] = { 0 };
		};
	}
	
	
}