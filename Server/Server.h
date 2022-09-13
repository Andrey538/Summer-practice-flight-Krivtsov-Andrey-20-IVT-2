#pragma once
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <stack>
#include "Log.h"
#include "User.h"
#include "Timer.h"
#include "Types.h"
#include "Lobby.h"
#include "Socket.h"
#include "Package.h"
#include "Requests.h"
#include "Registrator.h"

namespace Andrew
{
	class BaseServer
	{
	public:
		BaseServer(const int& domain, const int& service, const int& protocol, const int& port,
			const char* host, const int& backlog, const std::string& logPath)
		{
			log = new Log(logPath);

			WSADATA ws;
			int useless = WSAStartup(MAKEWORD(2, 2), &ws);

			listeningSocket = new ListeningSocket(domain, service, protocol, port, host, backlog);
			
		}
		~BaseServer()
		{
			delete log;
			delete listeningSocket;
		}

		virtual void launch() = 0;
	protected:
		virtual Package accepter(int sock) = 0;
		virtual Package handler(int sock, const Package& request) = 0;
		virtual void responder(int sock, const Package& response) = 0;
	protected:
		Log* log = nullptr;
		ListeningSocket* listeningSocket = nullptr;
	};

	class PicSharingServer : public BaseServer
	{
	public:
		PicSharingServer(const int& domain, const int& service, const int& protocol, const int& port, const char* host, const int& backlog, const std::string& logPath, 
			const int& maxUsers, const int& maxLobbies)
			:	BaseServer(domain, service, protocol, port, host, backlog, logPath),
				maxUsers(maxUsers),
				maxLobbies(maxLobbies),
				elapsedTime(timer.getElapsed())
		{
			launch();
		}
		virtual void launch() override
		{
			while (true)
			{
				int lsocket = listeningSocket->getSock();
				struct sockaddr_in address = listeningSocket->getAddress();
				int addrlen = sizeof(address);
				log->write("==== Starting server ====\n");
				int nsocket = accept(lsocket, (struct sockaddr*)&address, &addrlen);
				if (nsocket < 0)
				{
					log->write("==== accept failed with error: " + WSAGetLastError());
					log->write(" ====\n");
					closesocket(nsocket);
				}
				else
				{
					log->write("==== New connection, Sock: " + std::to_string(nsocket) + " ====\n");
				}
				std::thread t( [this, nsocket] {handleSocket(nsocket); });
				t.join();
			}
			log->write("==== Closing server ====\n");			
		}
	protected:
		void handleSocket(int sock)
		{
			while (true)
			{
				log->write("==== Waiting a connection ====\n");
				elapsedTime = timer.getElapsed();

				Package request = accepter(sock);
				Package response = handler(sock, request);
				responder(sock, response);

				log->write("==== Done ====\n");
			}
			
		}
		virtual Package accepter(int sock) override
		{
			Package request;
			int netres = recv(sock, (char*)&request, sizeof(request), 0);
			if (netres > 0)
			{
				log->write("==== Recv: " + std::to_string(netres) + " bytes ====\n");
			}
			else if (netres == 0)
			{
				log->write("==== Connection closed ====\n");
			}
			else
			{
				log->write("==== recv failed with error: " + WSAGetLastError());
				log->write(" ====\n");
				closesocket(sock);
			}
			return request;
		}
		virtual Package handler(int sock, const Package& request) override
		{
			log->write("==== Package: ");
			switch (request.type)
			{
			case PckgType::PingReq:
				log->write("Ping ====\n");
				return pingHandle();
			case PckgType::AuthReq:
				log->write("Auth ====\n");
				return authHandle();
			case PckgType::LobbyRegReq:
				log->write("Lobby registration ====\n");
				return lobbyRegHandle(request);
			case PckgType::LobbyConnectReq:
				log->write("Lobby connection ====\n");
				return lobbyConnectHandle(request);
			case PckgType::SendPicChunkReq:
				log->write("Sending pic chunk ====\n");
				return sendPicChunkHandle(request);
			case PckgType::GetPicChunkReq:
				log->write("Getting pic chunk ====\n");
				return getPicChunkHandle(request);
			default:
				log->write("Unknown ====\n");
				return pingHandle();
			}
		}
		virtual void responder(int sock, const Package& response) override
		{
			int netres = send(sock, (char*)&response, sizeof(response), 0);
			if (netres > 0)
			{
				log->write("==== Send: " + std::to_string(netres) + " bytes ====\n");
			}
			else if (netres == 0)
			{
				log->write("==== Connection closed ====\n");
			}
			else
			{
				log->write("==== send failed with error: " + WSAGetLastError());
				log->write(" ====\n");
				closesocket(sock);
			}
		}
	private:
		Timer timer;
		double elapsedTime = 0.0;

		u_long maxUsers, maxLobbies;

		Registrator<User> users;
		std::mutex usersMutex;

		Registrator<Lobby> lobbies;
		std::mutex lobbiesMutex;
	private:
		/** Package handlers 
		*/
		Package pingHandle()
		{
			Package response;
			response.type = PckgType::PingRes;
			
			Response::Ping payload = {
				.status = 1
			};
			
			memcpy(response.payload, (char*)&payload, sizeof(payload));
			return response;
		}
		Package authHandle()
		{
			Package response;
			response.type = PckgType::AuthRes;
			Response::Auth res;

			usersMutex.lock();
			if (users.id + 1 <= maxUsers)
			{
				u_long id = users.id++;
				User user(id);
				users.data[id] = user;

				res.status = 1;
				res.userId = htonl(user.id);				
				log->write("Successful auth\n");
			}
			else
			{
				res.status = 0;
				res.userId = htonl(0);
				log->write("Failed auth\n");
			}
			usersMutex.unlock();

			memcpy(response.payload, &res, sizeof(res));
			return response;
		}
		Package lobbyRegHandle(const Package& request)
		{
			Request::LobbyReg payload;
			memcpy(&payload, request.payload, sizeof(payload));
			payload.id		 = ntohl(payload.id);
			payload.dataSize = ntohl(payload.dataSize);
			payload.nameSize = ntohl(payload.nameSize);
			payload.chunks	 = ntohl(payload.chunks);

			Package response;
			response.type = PckgType::LobbyRegRes;
			Response::LobbyReg res;

			lobbiesMutex.lock();
			if (lobbies.id + 1 <= maxLobbies)
			{
				u_long id = lobbies.id++;
				Lobby lobby(id, payload.id, payload.dataSize, payload.chunks, payload.dataName);
				lobbies.data[id] = lobby;

				res.status = 1;
				res.lobbyId = htonl(id);
				log->write("Successful lobby registration\n");
			}
			else
			{
				res.status = 0;
				res.lobbyId = htonl(0);
				log->write("Failed lobby registration\n");
			}
			lobbiesMutex.unlock();

			memcpy(response.payload, (char*)&res, sizeof(res));
			return response;
		}
		Package lobbyConnectHandle(const Package& request)
		{
			Request::LobbyConnect payload;
			memcpy(&payload, request.payload, sizeof(payload));
			payload.id = ntohl(payload.id);
			payload.lobbyId = ntohl(payload.lobbyId);

			Package response;
			response.type = PckgType::LobbyConnectRes;
			Response::LobbyConnect res;

			if (!lobbies.data.count(payload.lobbyId))
			{
				res.status = 0;
				log->write("Lobby doesnt exist\n");
				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			Lobby& lobby = lobbies.data[payload.lobbyId];
			if (!lobby.uploaded)
			{
				log->write("Picture is not uploaded yet!\n");
				res.status = 0;

				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			u_long nameSize = strlen(lobby.dataName.c_str());
			res.status = 1;
			res.dataSize = htonl(lobby.buffer.size);
			res.nameSize = htonl(nameSize);
			memcpy(res.dataName, &lobby.dataName, nameSize);

			memcpy(response.payload, (char*)&res, sizeof(res));
			return response;
		}
		Package sendPicChunkHandle(const Package& request)
		{
			Request::SendPicChunk payload;
			memcpy(&payload, request.payload, sizeof(payload));
			payload.id		= ntohl(payload.id);
			payload.lobbyId = ntohl(payload.lobbyId);
			payload.chunkId = ntohl(payload.chunkId);
			payload.size	= ntohl(payload.size);
			payload.offset	= ntohl(payload.offset);

			Package response;
			response.type = PckgType::SendPicChunkRes;
			Response::SendPicChunk res;

			if (!lobbies.data.count(payload.lobbyId))
			{
				res.status = 0;
				log->write("Lobby doesnt exist\n");
				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			Lobby& lobby = lobbies.data.at(payload.lobbyId);

			if (payload.id != lobby.ownerId)
			{
				log->write("Not the owner tried to send data\n");
				res.status = 0;
				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			try
			{
				lobbiesMutex.lock();
				
				memcpy(lobby.buffer.data + payload.offset, payload.bin, payload.size);
				res.status = 1;
				res.chunkId = htonl(payload.chunkId);

				/** Uploading end
				*/
				if (payload.chunkId == lobby.chunks - 1) lobby.uploaded = 1;
				lobbiesMutex.unlock();

				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			catch(const std::exception& e)
			{
				res.status = 0;
				res.chunkId = htonl(payload.chunkId);

				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
		}
		Package getPicChunkHandle(const Package& request)
		{
			Request::GetPicChunk payload;
			memcpy(&payload, request.payload, sizeof(payload));
			payload.lobbyId = ntohl(payload.lobbyId);
			payload.chunkId = ntohl(payload.chunkId);

			Package response;
			response.type = PckgType::GetPicChunkRes;
			Response::GetPicChunk res;

			if (!lobbies.data.count(payload.lobbyId))
			{
				res.status = 0;
				log->write("Lobby doesnt exist\n");
				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			Lobby& lobby = lobbies.data[payload.lobbyId];
			try
			{				
				u_long common = sizeof(Request::SendPicChunk::bin),
					offset = payload.chunkId * common;
				u_long size = getChunkSize(lobby.buffer.size, common, lobby.chunks, payload.chunkId);

				lobbiesMutex.lock();
				memcpy(res.bin, lobby.buffer.data + offset, size);
				lobbiesMutex.unlock();

				res.status = 1;
				res.size = htonl(size);
				res.offset = htonl(offset);
				res.chunkId = htonl(payload.chunkId);				

				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
			catch (const std::exception& e)
			{
				res.status = 0;
				res.chunkId = htonl(payload.chunkId);

				memcpy(response.payload, (char*)&res, sizeof(res));
				return response;
			}
		}

		u_long getChunkSize(u_long whole, u_long common, u_long chunks, u_long id)
		{
			u_long chunkSize = id == chunks - 1
				? whole - common * id : common;
			return chunkSize;
		}

		void updateLobbiesLife()
		{
			std::stack<u_long> toDel;
			for (auto& data : lobbies.data) {
				Lobby& lobby = data.second;
				lobby.lifetime += elapsedTime;
				if (lobby.lifetime > 300) toDel.push(data.first);
			}
			while (!toDel.empty())
			{
				u_long del = toDel.top();
				toDel.pop();
				lobbies.data.erase(del);
			}
		}
		void shutdownSend(int sock)
		{
			int res = shutdown(sock, SD_SEND);
			if (res < 0)
			{
				log->write("==== shutdown failed with error: " + WSAGetLastError());
				log->write(" ====\n");

				closesocket(sock);
			}
		}
		void shutdownRecv(int sock)
		{
			int res = shutdown(sock, SD_RECEIVE);
			if (res < 0)
			{
				log->write("==== shutdown failed with error: " + WSAGetLastError());
				log->write(" ====\n");

				closesocket(sock);
			}
		}
	};
}

