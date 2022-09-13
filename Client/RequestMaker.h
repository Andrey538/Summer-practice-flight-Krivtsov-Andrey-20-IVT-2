#pragma once
#include "Package.h"
#include "Requests.h"

namespace Andrew
{
	class RequestMaker
	{
	public:
		RequestMaker() = default;

		Package auth()
		{
			Package request;
			request.type = PckgType::AuthReq;

			Request::Auth payload;

			memcpy(&request.payload, &payload, sizeof(payload));
			return request;
		}
		Package lobbyReg(u_long id, u_long dataSize, u_long nameSize, u_long chunks,  const char* dataName)
		{
			Package request;
			request.type = PckgType::LobbyRegReq;

			Request::LobbyReg payload =
			{
				.id			= htonl(id),
				.dataSize	= htonl(dataSize),
				.nameSize	= htonl(nameSize),
				.chunks		= htonl(chunks),
			};			
			memcpy(payload.dataName, dataName, nameSize);

			memcpy(&request.payload, &payload, sizeof(payload));
			return request;
		}
		Package lobbyConnect(u_long id, u_long lobbyId)
		{
			Package request;
			request.type = PckgType::LobbyConnectReq;

			Request::LobbyConnect payload =
			{
				.id			= htonl(id),
				.lobbyId	= htonl(lobbyId),
			};

			memcpy(&request.payload, &payload, sizeof(payload));
			return request;
		}
		Package sendPicChunk(u_long id, u_long lobbyId, u_long chunkId, u_long size, u_long offset, const char* buffer)
		{
			Package request;
			request.type = PckgType::SendPicChunkReq;

			Request::SendPicChunk payload =
			{
				.id			= htonl(id),
				.lobbyId	= htonl(lobbyId),
				.chunkId	= htonl(chunkId),
				.size		= htonl(size),
				.offset		= htonl(offset),
			};
			memcpy(payload.bin, buffer + offset, size);

			memcpy(&request.payload, &payload, sizeof(payload));
			return request;
		}
		Package getPicChunk(u_long lobbyId, u_long chunkId)
		{
			Package request;
			request.type = PckgType::GetPicChunkReq;

			Request::GetPicChunk payload =
			{
				.lobbyId = htonl(lobbyId),
				.chunkId = htonl(chunkId),
			};

			memcpy(&request.payload, &payload, sizeof(payload));
			return request;
		}
	};
}