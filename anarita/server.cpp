#include "server.hpp"

Server::Server(int port)
{
	_port = port;
	_serverFd = 0;

}


Server::~Server()
{

}
void Server::create_socket()
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
}

void Server::start()
{
	create_socket();

	std::vector <struct pollfd> fds;

	struct pollfd server_poll;
	std::memset(&server_poll, 0, sizeof(server_poll));
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
		size_t size = fds.size();
		size_t i = 0;
		while(i < size)
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
					struct pollfd client_poll;
					std::memset(&client_poll, 0, sizeof(client_poll));
					client_poll.fd = clientFd;
					client_poll.events = POLLIN;
					fds.push_back(client_poll);
				}
				i++;
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
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						size--;
						continue;
					}
					else if (bytesRead == 0)
					{
						std::cerr << "Client disconnected "<< std::endl;
						close(fds[i].fd);
						fds.erase(fds.begin() + i);
						size--;
						continue;
					}
					buffer[bytesRead] = '\0';
					std::cout << "Message from client: " << buffer;
					i++;
				}
				else
					i++;
				
			}
		}
	}

	close(_serverFd);
	
}


int main(int ac, char **av)
{
	if(ac != 2)
		return 1;
	Server srv(std::atoi(av[1]));
	srv.start();
	return 0;
	
}