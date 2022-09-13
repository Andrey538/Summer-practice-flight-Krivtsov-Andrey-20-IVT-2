#pragma once
#include "Package.h"
#include "Requests.h"

namespace Andrew
{
	/** Casts network data to host data
	*/
	class ResponseExtractor
	{
	public:
		ResponseExtractor() = default;

		Response::Auth auth(char* const* payload)
		{
			Response::Auth res;
			memcpy(&res, payload, sizeof(res));

			res.userId = ntohl(res.userId);

			return res;
		}
		Response::LobbyReg lobbyReg(char* const* payload)
		{
			Response::LobbyReg res;
			memcpy(&res, payload, sizeof(res));

			res.lobbyId = ntohl(res.lobbyId);

			return res;
		}
		Response::LobbyConnect lobbyConnect(char* const* payload)
		{
			Response::LobbyConnect res;
			memcpy(&res, payload, sizeof(res));

			res.dataSize = ntohl(res.dataSize);
			res.nameSize = ntohl(res.nameSize);

			return res;
		}
		Response::SendPicChunk sendPicChunk(char* const* payload)
		{
			Response::SendPicChunk res;
			memcpy(&res, payload, sizeof(res));

			res.chunkId = ntohl(res.chunkId);

			return res;
		}
		Response::GetPicChunk getPicChunk(char* const* payload)
		{
			Response::GetPicChunk res;
			memcpy(&res, payload, sizeof(res));

			res.chunkId = ntohl(res.chunkId);
			res.size	= ntohl(res.size);
			res.offset	= ntohl(res.offset);

			return res;
		}
	};
}