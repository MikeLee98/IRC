#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

void	grace_err_exit(int fd){
	close (fd);
	exit(1);
}
#define MAX_CLIENTS 1024
typedef struct poll_fds{
	struct pollfd	pollfds[MAX_CLIENTS];
	int		fd_size;
	std::string	fd_buffers[MAX_CLIENTS];
}	poll_fds;

int	update_poll_fds(poll_fds *pfds, int fd, short events)
{
	int& size = pfds->fd_size;
	if (size >= MAX_CLIENTS) return 0;
	pfds->pollfds[size].fd = fd;
	pfds->pollfds[size].events = events;
	size++;
	std::cout << "Poll size: " << pfds->fd_size << std::endl;
	return 1;
}
void	send_client_message(poll_fds& pfds, int index, std::string message){
	int bsent = send(pfds.pollfds[index].fd, message.c_str(), message.size(), 0); 
	(void)bsent;
	return ;
}
void	handle_server_connection(poll_fds& pfds)
{
	int new_fd = accept(pfds.pollfds[0].fd, NULL, NULL);
	if (new_fd == -1){
		std::cerr << "failed to handle an incomming connection.." << std::endl;
		return ;
	}
	if (update_poll_fds(&pfds, new_fd, POLLIN))
	{
		std::cerr << "failed to accept new client: client max limit reached" << std::endl;
		close (new_fd);
		return ;
	}
	// send happy message !!
	send_client_message(pfds, pfds.fd_size - 1, "Welcome to our IRC server!");
}

void	handle_poll_event(poll_fds& pfds)
{
	int &size = pfds.fd_size;
	if (pfds.pollfds[0].revents & POLLIN){
		// handle server connection
		handle_server_connection(pfds);
	}
	for (int i = 1; i < size; i++){
		if (!(pfds.pollfds[i].revents & (POLLIN | POLLHUP))) continue ;
		// handle client request
	}
}

int	main(void){
	struct addrinfo	hint;
	struct addrinfo	*res;
	memset(&hint, 0, sizeof hint);
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, "7777", &hint, &res) != 0)
	{
		std::cerr << "There's an error!" << std::endl; 
		exit(1);
	}
	struct addrinfo *temp = res;
	int sockfd = -1;
	while (temp){
		sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
		if (sockfd == -1){
			std::cerr << "Socket: error" << std::endl;
			temp = temp->ai_next;
			continue ;
		}
		if (bind(sockfd, temp->ai_addr, temp->ai_addrlen) == -1){
			std::cerr << "bind: error" << std::endl;
			temp = temp->ai_next;
			close(sockfd);
			continue ;
		}
		break ;

	}
	freeaddrinfo(res);
	if (!temp) {
		std::cerr << "bind: failed to bind" << std::endl;
		grace_err_exit(sockfd);
	}
	if (listen(sockfd, 10) == -1){
		std::cerr << "listen: error" << std::endl; 
		grace_err_exit(sockfd);
	}
	poll_fds pfds;
	memset((void *)(&pfds), 0, sizeof(pfds));
	if (!update_poll_fds(&pfds, sockfd, POLLIN)){
		std::cerr << "max fds limit reached! unable to start the server." << std::endl;
		return 1;
	}
	std::cout << "poll: server is listening to incomming connections." << std::endl;

	while (true)
	{
		int nevents = poll(pfds.pollfds, pfds.fd_size, 1000);
		if (!nevents){
			std::cout << "no events found.." << std::endl;
		}
		else {
			std::cout << "[~] event found" << std::endl;
			handle_poll_event(pfds);
		}
	}

	return 0;
}
