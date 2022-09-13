#include "Client.h"

//==== config ====//
int domain = AF_INET,
service = SOCK_STREAM,
protocol = 0,
port = 83,
backlog = 16,
maxUsers = 256;
const char* host = "127.0.0.1";
std::string logPath = "./server.txt";
//================//

int main(int argc, char** argv)
{
	Andrew::PicSharingClient client(
		domain,
		service,
		protocol,
		port,
		host
	);

	return 0;
}
