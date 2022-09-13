#pragma once
#include <queue>
#include "File.h"
#include "User.h"
#include "UserIO.h"
#include "Socket.h"
#include "Package.h"
#include "Requests.h"
#include "RequestMaker.h"
#include "ResponseExtractor.h"

namespace Andrew
{
	class PicSharingClient
	{
	public:
		PicSharingClient(const int& domain, const int& service, const int& protocol,
			const int& port, const char* host) 
			:	wsaStartup(WSAStartup(MAKEWORD(2, 2), &ws)),
				connectingSocket(domain, service, protocol, port, host),				
				io(PayloadSize)
		{
			if (wsaStartup < 0) exit(EXIT_FAILURE);

			launch();
		}
	private:
		/*
		*	at start constructor call WSAStartup()
		*	then socketconstructor. IMPORTANT!!!
		*/
		int wsaStartup = -1;
		ConnectingSocket connectingSocket;
		/** THESE ^^^
		*/
		UserIO io;
		User user;
		WSADATA ws;
		
		bool closing = 0;

		RequestMaker makeRequest;
		ResponseExtractor extractResponse;
	private:
		void launch()
		{
			io.writeln("==== App starting ====");
			authOrExit();

			while (!closing)
			{
				run();
			}
			io.writeln("==== App closing ====");
		}
		void run()
		{
			io.writeln("==== Options: ====");
			
			io.writeln("[1] - Upload   picture");
			io.writeln("[2] - Download picture");
			io.writeln("[3] - exit");

			switch (io.getOption(1, 4))
			{
			case 1:
				uploadPicture();
				break;
			case 2:
				downloadPicture();
				break;
			case 3:
				closing = true;
				break;
			default:
				io.writeln("==== Unknown option ====");
			}
			
		}
		void authOrExit()
		{
			bool valid = 0;
			do
			{
				io.writeln("==== Auth starting ====");

				sender(makeRequest.auth());

				Package response = accepter();
				Response::Auth payload = extractResponse.auth(response.payload);
				user.auth	= payload.status;
				user.id		= payload.userId;

				if (user.auth)
				{
					io.writeln("==== Successful auth ====");

					io.write("==== Your Id : ");
					io.write(std::to_string(user.id));
					io.write(" ====\n");

					valid = 1;

					io.writeln("==== Auth ending ====");
				}
				else
				{
					io.writeln("==== Failed auth ====");
					io.writeln("==== Server is overload ====");

					io.writeln("==== Options: ====");
					io.writeln("[1] - auth");
					io.writeln("[2] - exit");

					switch (io.getOption(1, 3))
					{
					case 1:
						continue;
						break;
					case 2:
						closing = true;
						break;
					default:
						io.writeln("==== Unknown option ====");
					}
				}
			} while (!valid && !closing);
		}
		void uploadPicture()
		{
			std::string path, name;
			File picture;
			getPictureFromDisk(path, name, picture);
			u_long picSize = picture.getSize();
			u_long common  = sizeof(Request::SendPicChunk::bin);
			u_long chunks  = getChunksCount(picSize, common);

			Response::LobbyReg lobbyRegRes = createLobby(picture.getSize(), chunks, name.c_str());
			if (lobbyRegRes.status == 0)
			{
				io.writeln("Failed lobby registration!\nTry again later!");
				return;
			}
			u_long lobbyId = lobbyRegRes.lobbyId;
			io.writeln("Lobby created, id: " + std::to_string(lobbyId));
			io.writeln("Uploading started");

			int loadstep = 20, loadprogress = 0;
			for (int chunkId = 0; chunkId < chunks; chunkId++)
			{
				bool sent = 0;
				while (!sent)
				{
					sender(makeRequest.sendPicChunk(
						user.id,
						lobbyId,
						chunkId,
						getChunkSize(picSize, common, chunks, chunkId),
						getChunkOffset(common, chunkId),
						picture.getBuffer()
					));
					Response::SendPicChunk payload = extractResponse.sendPicChunk(accepter().payload);

					if (payload.status == 1) sent = 1;
				}
				
				if (chunkId == (int)(loadprogress / 100.0 * (chunks - 1)))
				{
					io.write("Uploading: " + std::to_string(loadprogress) + "%\n");
					loadprogress += (loadprogress + loadstep <= 100) ? loadstep : 100 - loadprogress;
				}
			}

		}
		void downloadPicture()
		{
			int lobbyId = io.askForInt("Type lobby id: ");
			std::string path;
#ifdef _DEBUG
			path = "C:/assets/test2.jpg";
#else
			io.askForString("Type path to save: ", path);
#endif
			

			Response::LobbyConnect lobbyConnectRes = joinLobby(lobbyId);
			if (lobbyConnectRes.status == 0)
			{
				io.writeln("Wrong!\nTry again later");
				return;
			}
			std::string name(lobbyConnectRes.dataName, lobbyConnectRes.nameSize);
			io.writeln("Downloading: " + name);
			u_long picSize = lobbyConnectRes.dataSize;
			u_long common = sizeof(Request::SendPicChunk::bin);
			u_long chunks = getChunksCount(picSize, common);

			char* tmp = new char[picSize];

			int loadstep = 20, loadprogress = 0;
			for (int chunkId = 0; chunkId < chunks; chunkId++)
			{
				bool downloaded = 0;
				while (!downloaded)
				{
					sender(makeRequest.getPicChunk(						
						lobbyId,
						chunkId
					));
					Response::GetPicChunk payload = extractResponse.getPicChunk(accepter().payload);

					/** putting pic chunk to memory 
					*/
					memcpy(tmp + payload.offset, payload.bin, payload.size);

					if (payload.status == 1) downloaded = 1;
				}
				if (chunkId == (int)(loadprogress / 100.0 * (chunks -1)))
				{
					io.write("Downloding: " + std::to_string(loadprogress) + "%\n");
					loadprogress += (loadprogress + loadstep <= 100) ? loadstep : 100 - loadprogress;
				}
			}

			File picture(ByteBuffer(tmp, picSize));
			picture.bWrite(path);
		}

		Response::LobbyReg createLobby(u_long picSize, u_long chunks, const char* name)
		{
			io.writeln("Start creating a lobby");
			sender(makeRequest.lobbyReg(
				user.id,
				picSize,
				strlen(name),
				chunks,
				name
			));
			return extractResponse.lobbyReg(accepter().payload);
		}
		Response::LobbyConnect joinLobby(u_long lobbyId)
		{
			io.writeln("Start joining a lobby");
			sender(makeRequest.lobbyConnect(
				user.id,
				lobbyId
			));
			return extractResponse.lobbyConnect(accepter().payload);
		}

		void getPicturePathAndName(std::string& path, std::string& name)
		{
			bool valid = 0;
			do
			{
#ifdef _DEBUG

				path = "C:/assets/test.jpg";
				name = " test.jpg";
#else
				io.askForString("Type picture path on computer: ", path);
				name = path.find('/') ? path.substr(path.find_last_of('/') + 1)
					: path.substr(path.find_last_of('\\') + 1);
				
#endif
				valid = strlen(name.c_str()) <= sizeof(Request::LobbyReg::dataName);
				if (!valid) io.writeln("Too long picture name(not path, name)!\nTry again!");

			} while (!valid);			
		}
		void getPictureFromDisk(std::string& path, std::string& name, File& picture)
		{
			bool valid = 0;
			do
			{
				getPicturePathAndName(path, name);
				picture.bRead(path);
				valid = picture.getSize() > 0;
				if (!valid ) io.writeln("Try again!");
			} while (!valid);
			
		}

		Package accepter()
		{
			Package response;
			int netres = recv(connectingSocket.getSock(), (char*)&response, sizeof(response), 0);
			if (netres > 0)
			{
#ifdef _DEBUG
				io.write("==== Recv: " + std::to_string(netres) + " bytes ====\n");
#endif
			}
			else if (netres == 0)
			{
#ifdef _DEBUG
				io.write("==== Connection closed ====\n");
#endif
			}
			else
			{
#ifdef _DEBUG
				io.write("==== recv failed with error: " + WSAGetLastError());
				io.write(" ====\n");
#endif
				closesocket(connectingSocket.getSock());
			}
			
			return response;
		}
		void sender(const Package& request)
		{
			int netres = send(connectingSocket.getSock(), (char*)&request, sizeof(request), 0);
			if (netres > 0)
			{
#ifdef _DEBUG
				io.write("==== Send: " + std::to_string(netres) + " bytes ====\n");
#endif
			}
			else if (netres == 0)
			{
#ifdef _DEBUG
				io.write("==== Connection closed ====\n");
#endif
			}
			else
			{
#ifdef _DEBUG
				io.write("==== send failed with error: " + WSAGetLastError());
				io.write(" ====\n");
#endif
				closesocket(connectingSocket.getSock());
			}			
		}
		void shutdownSend()
		{
			int netres = shutdown(connectingSocket.getSock(), SD_SEND);
			if (netres < 0)
			{
#ifdef _DEBUG
				io.write("==== shutdown failed with error: " + WSAGetLastError());
				io.write(" ====\n");
#endif
				closesocket(connectingSocket.getSock());
			}
		}
		void shutdownRecv()
		{
			int netres = shutdown(connectingSocket.getSock(), SD_RECEIVE);
			if (netres < 0)
			{
#ifdef _DEBUG
				io.write("==== shutdown failed with error: " + WSAGetLastError());
				io.write(" ====\n");
#endif
				closesocket(connectingSocket.getSock());
			}
		}
		/** Picture chunks handlers
		*/
		u_long getChunksCount(u_long size, u_long payloadSize)
		{
			u_long res = size / payloadSize;
			if (res % payloadSize > 0) res++;
			return res;
		}
		u_long getChunkSize(u_long whole, u_long common, u_long chunks, u_long chunkId)
		{
			u_long chunkSize = chunkId == chunks - 1
				? whole - common * chunkId : common;
			return chunkSize;
		}
		u_long getChunkOffset(u_long common, u_long chunkId)
		{
			return chunkId * common;
		}
	};
}