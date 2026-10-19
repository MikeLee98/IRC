#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <csignal>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void	grace_err_exit(int fd){
	close (fd);
	exit(1);
}
#define MAX_CLIENTS 1024
typedef struct poll_fds{
	struct pollfd	pollfds[MAX_CLIENTS];
	int		fd_size;
	std::string	fd_buffers[MAX_CLIENTS];
	poll_fds() : fd_size(0) { memset(pollfds, 0, sizeof pollfds); }
}	poll_fds;

int	update_poll_fds(poll_fds *pfds, int fd, short events)
{
	int& size = pfds->fd_size;
	if (size >= MAX_CLIENTS) return 0;
	pfds->pollfds[size].fd = fd;
	pfds->pollfds[size].events = events;
	pfds->pollfds[size].revents = 0;
	size++;
	std::cout << "Poll size: " << pfds->fd_size << std::endl;
	return 1;
}
void	client_buffer_flush(poll_fds& pfds, int index){
	std::string& tmsg = pfds.fd_buffers[index];
	size_t bsent = send(pfds.pollfds[index].fd, tmsg.c_str(), tmsg.size(), 0); 
	if (bsent > 0) tmsg.erase(0, bsent);
	if (tmsg.empty())
		pfds.pollfds[index].events &= ~POLLOUT;
	else 
		pfds.pollfds[index].events |= POLLOUT;
	
	return ;
}
void	send_client_message(poll_fds& pfds, int index, const std::string& message){
	pfds.fd_buffers[index] += message;
	client_buffer_flush(pfds, index);
}
void	handle_server_connection(poll_fds& pfds)
{
	int new_fd = accept(pfds.pollfds[0].fd, NULL, NULL);
	if (new_fd == -1){
		std::cerr << "failed to handle an incomming connection.." << std::endl;
		return ;
	}
	if (!update_poll_fds(&pfds, new_fd, POLLIN))
	{
		std::cerr << "failed to accept new client: client max limit reached" << std::endl;
		close (new_fd);
		return ;
	}
	// send happy message !!
	send_client_message(pfds, pfds.fd_size - 1, "Welcome to our IRC server!\n");
}

void	close_error_socket(poll_fds& pfds, int i){
	int& fd_size = pfds.fd_size;
	close(pfds.pollfds[i].fd);
	pfds.pollfds[i] = pfds.pollfds[fd_size - 1];
	pfds.fd_buffers[i] = pfds.fd_buffers[fd_size - 1];
	pfds.fd_buffers[fd_size - 1].clear();
	fd_size--;
	return ;
}

void	handle_poll_event(poll_fds& pfds)
{
	int &size = pfds.fd_size;
	if (pfds.pollfds[0].revents & POLLIN){
		// handle server connection
		handle_server_connection(pfds);
	}
	for (int i = 1; i < size; i++){
		if (!pfds.pollfds[i].revents) continue ;
		if (pfds.pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
			close_error_socket(pfds, i);
			i--;
			continue ;
		}
		if (pfds.pollfds[i].revents & POLLOUT) client_buffer_flush(pfds,i);
		//if (pfds.pollfds[i].revents & POLLIN) handle_client_input(pfds, i);

		// handle client request
	}
}

int	main(void){
	signal(SIGNUM, action);
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
	if (!update_poll_fds(&pfds, sockfd, POLLIN)){
		std::cerr << "max fds limit reached! unable to start the server." << std::endl;
		return 1;
	}
	std::cout << "poll: server is listening to incomming connections." << std::endl;

	while (true)
	{
		int nevents = poll(pfds.pollfds, pfds.fd_size, 1000);
		if (nevents == -1){
			if (errno == EINTR) continue ;
			perror("poll");
			exit(1);
		}
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
