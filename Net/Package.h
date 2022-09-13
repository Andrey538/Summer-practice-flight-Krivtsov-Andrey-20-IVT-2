#pragma once
#include "Types.h"

namespace Andrew
{
	const int PayloadSize = 512;

	enum class PckgType
	{
		PingReq,
		PingRes,

		AuthReq,
		AuthRes,

		LobbyRegReq,
		LobbyRegRes,

		LobbyConnectReq,
		LobbyConnectRes,

		SendPicChunkReq,
		SendPicChunkRes,

		GetPicChunkReq,
		GetPicChunkRes,
	};

	struct Package
	{
		PckgType type;
		char* payload[PayloadSize];
	};
}
