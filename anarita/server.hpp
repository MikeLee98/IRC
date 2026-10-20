#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <poll.h>
#include <vector>

class Server
{
	private:
		int _serverFd;
		int _port;
	public:
		Server(int port);
		~Server();
		void start();
};

#endif