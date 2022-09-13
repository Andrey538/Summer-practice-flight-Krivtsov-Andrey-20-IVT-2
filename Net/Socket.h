#pragma once
#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

namespace Andrew
{
	class PlainSocket
	{
	public:
		PlainSocket(const int& domain, const int& service, const int& protocol, const int& port, const char* host)
		{
			address.sin_family = domain;
			address.sin_port = htons(port);
			inet_pton(domain, host, &address.sin_addr);
			sock = socket(domain, service, protocol);
			test(sock);
		}

		const struct sockaddr_in& getAddress() const { return address; }
		const int& getConnection() const { return connection; }
		const int& getSock() const { return sock; }
	protected:
		struct sockaddr_in address;
		int connection;
		int sock;
	protected:
		void test(int i)
		{
			if (i < 0)
			{
				perror("Failed connection\n");
				exit(EXIT_FAILURE);
			}
		}
	};

	class BindingSocket : public PlainSocket
	{
	public:
		BindingSocket(const int& domain, const int& service, const int& protocol, const int& port,
			const char* host)
			: PlainSocket(domain, service, protocol, port, host)
		{
			connection = bind(sock, (struct sockaddr*)&address, sizeof(address));
			test(connection);
		}
	};

	class ConnectingSocket : public PlainSocket
	{
	public:
		ConnectingSocket(const int& domain, const int& service, const int& protocol, const int& port,
			const char* host)
			: PlainSocket(domain, service, protocol, port, host)
		{
			connection = connect(sock, (struct sockaddr*)&address, sizeof(address));
			test(connection);
		}
	};

	class ListeningSocket : public BindingSocket
	{
	public:
		ListeningSocket(const int& domain, const int& service, const int& protocol, const int& port, 
			const char* host, const int& backlog)
			: BindingSocket(domain, service, protocol, port, host), backlog(backlog)
		{
			startListening();
			test(listening);
		}
		void startListening()
		{
			listening = listen(sock, backlog);
		}
	protected:
		int backlog;
		int listening;
	};
}