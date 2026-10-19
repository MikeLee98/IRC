#include "server.hpp"

Server::Server(int port)
{
	_port = port;
	_serverFd = 0;

}


Server::~Server()
{

}

void Server::start()
{
	struct sockaddr_in addr;
	
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if(_serverFd == -1)
	{
		std::cerr << "Error: socket failed: " << std::strerror(errno) << std::endl;
		return ;
	}

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	

	if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "Error: bind failed: " << std::strerror(errno) << std::endl;
		close(_serverFd);
		return ;
	}
	if (listen(_serverFd, 10) == -1)
	{
		std::cerr << "Error: listen failed. " << std::strerror(errno) << std::endl;
		close(_serverFd);
		return ;
	}
	std::cout << "Server is listening on port " << _port << std::endl;

	std::vector <struct pollfd> fds;

	struct pollfd server_poll = {};
	server_poll.fd = _serverFd;
	server_poll.events = POLLIN;
	fds.push_back(server_poll);

	while(true)
	{
		int pollres = poll(fds.data(), fds.size(), -1);
		if (pollres == -1)
		{
			std::cerr << "Error: poll error " << std::strerror(errno) << std::endl;
			break;
		}
		for (size_t i = 0; i < fds.size(); i++)
		{
			if (i == 0)
			{
				if (fds[i].revents & POLLIN)
				{
					std::cout << "New connection pending" << std::endl;
					int clientFd = accept(fds[0].fd, NULL, NULL);
					if (clientFd == -1)
					{
						std::cerr << "Error: accept failed: "<< std::strerror(errno) << std::endl;
						return;
					}
					struct pollfd client_poll = {};
					client_poll.fd = clientFd;
					client_poll.events = POLLIN;
					fds.push_back(client_poll);
				}	
			}
			else
			{
				if (fds[i].revents & POLLIN)
				{
					char buffer[1024];
					int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
					if (bytesRead == -1)
					{
						std::cerr << "Error: recv failed: "<< std::strerror(errno) << std::endl;
						return;
					}
					else if (bytesRead == 0)
					{
						std::cerr << "Client disconnected "<< std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						i--;
						continue;
					}
					buffer[bytesRead] = '\0';
					std::cout << "Message from client: " << buffer;
				}
				
			}
		}
	}
	
	
	/*else if (serverPoll.revents & POLLERR)
		std::cout << "Error: Descriptor error" << std::endl;
	else if(serverPoll.revents & POLLHUP)
		std::cout << "Error: Connection terminated or closed" << std::endl;
	else if (serverPoll.revents & POLLNVAL)
		std::cout << "Error: Not a valid descriptor" << std::endl;*/

	
	;

	
	
	close(clientFd);
	close(_serverFd);
	
}


int main(int ac, char **av)
{
	if(ac != 2)
		return 1;
	Server srv(atoi(av[1]));
	srv.start();
	return 0;
	
}