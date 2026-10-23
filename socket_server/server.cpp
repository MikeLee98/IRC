#include "server.hpp"

std::map<int, Client> clients;
std::vector<pollfd>	pollfds;


void	grace_err_exit(int fd){
	close (fd);
	exit(1);
}

int	update_poll_fds(std::vector<pollfd>& pfds, int fd, short events)
{
	pollfd p;
	p.fd = fd;
	p.events = events;
	p.revents = 0;
	clients[fd] = Client();
	pfds.push_back(p);
	return 1;
}

enum BUFFER_TYPE {
	BUFFERIN,
	BUFFEROUT
};

void	close_error_socket(std::vector<pollfd>& pfds, long unsigned int i){
	long unsigned int fd_size = pfds.size();
	clients.erase(pfds[i].fd);
	close(pfds[i].fd);
	pfds[i] = pfds[fd_size - 1];
	pfds.pop_back();
	return ;
}
int	get_fd_client_check(std::vector<pollfd>& pfds, long unsigned index)
{
	int fd = pfds[index].fd;
	std::map<int, Client>::iterator it = clients.find(fd);	
	if (it == clients.end()) // means that it doesn't exist in the clients map!
	{
		close_error_socket(pfds, index);
		std::cerr << "Dropping Client FD: " << fd << ": doesnt exists in clients" << std::endl;
		return -1;
	}
	return (fd);
}

static int	append_client_buffer(std::vector<pollfd>& pfds, long unsigned int index, const std::string& message, BUFFER_TYPE btype)
{
	int fd = get_fd_client_check(pfds, index);
	if (fd == -1) return 0;
	Client& c = clients[fd];
	if (btype == BUFFERIN)
		c.buffer_in += message;
	else
		c.buffer_out += message;
	return 1;
}
static int	get_buffer_out(std::string **msg_addr, std::vector<pollfd>& pfds, int index)
{
	int fd = get_fd_client_check(pfds, index);
	if (fd == -1) return 0;
	Client& c = clients[fd];
	*msg_addr = &(c.buffer_out);
	return 1;
}

void	client_buffer_flush(std::vector<pollfd>& pfds, long unsigned int index){
	std::string *msg_addr = NULL;
	if (!get_buffer_out(&msg_addr, pfds, index)) return ;
	std::string& tmsg = *msg_addr;
	size_t bsent = send(pfds[index].fd, tmsg.c_str(), tmsg.size(), 0); 
	if (bsent > 0) tmsg.erase(0, bsent);
	if (tmsg.empty())
		pfds[index].events &= ~POLLOUT;
	else 
		pfds[index].events |= POLLOUT;
	return ;
}
void	send_client_message(std::vector<pollfd>& pfds, int index, const std::string& message){

	if (!append_client_buffer(pfds, index, message, BUFFEROUT)) return ;
	client_buffer_flush(pfds, index);
}
void	handle_server_connection(std::vector<pollfd>& pfds)
{
	int new_fd = accept(pfds[0].fd, NULL, NULL);
	if (new_fd == -1){
		std::cerr << "failed to handle an incomming connection.." << std::endl;
		return ;
	}
	if (!update_poll_fds(pfds, new_fd, POLLIN))
	{
		std::cerr << "failed to accept new client: client max limit reached" << std::endl;
		close (new_fd);
		return ;
	}
	// send happy message !!
	send_client_message(pfds, pfds.size() - 1, "Welcome to our IRC server!\n");
}

std::string	extract_complete_buffer(std::string& buffer)
{
	std::string output;
	size_t idx = buffer.find("\n");
	int has_r = (idx && buffer[idx - 1] == '\r');
	output = buffer.substr(0, idx - has_r);
	buffer.erase(0, idx + 1);
	return output;
}

void	client_broadcast_output(std::vector<pollfd>& pfds, int idx, std::string msg){
	std::cout << clients[pfds[idx].fd].name << " sent: " << msg << std::endl;
	msg += "\r\n";
	for (long unsigned int j = 1; j < pfds.size(); j++)
	{
		if (idx == (int)j) continue ;
		if (!append_client_buffer(pfds, j, msg, BUFFEROUT)){
			j--;
			continue ;
		}
		client_buffer_flush(pfds, j);
	}
	return ;
}

int	handle_client_command(std::vector<pollfd>& pfds, int index, const std::string buffer)
{
	(void)pfds;
	(void)index;
	(void)buffer;
	return 0;
}
static bool	is_buffer_complete(std::string bufferin)
{
	return (bufferin.find("\n") != std::string::npos);
}

void	handle_client_input(std::vector<pollfd> &pfds, long unsigned int i)
{
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	int bread = recv(pfds[i].fd, buffer, sizeof(buffer), 0);
	if (bread <= 0)
	{
		if (bread < 0)
			perror("rec err");
		else
			std::cout << "User closed the connection :" << pfds[i].fd << std::endl;
		close_error_socket(pfds, i);
		return ;
	}
	buffer[bread] = 0;
	if (!append_client_buffer(pfds, i, buffer, BUFFERIN)) return ;
	int fd = -1;
	if (is_buffer_complete(clients[pfds[i].fd].buffer_in))
	{
		fd = get_fd_client_check(pfds, i);
		if (fd == -1) return ; 
		std::string complete_buff = extract_complete_buffer(clients[fd].buffer_in);
		if (complete_buff.empty()) return ; 
	       	//if (handle_client_command(pfds, i, complete_buff)) return ;
		client_broadcast_output(pfds, i, complete_buff);
	}
	return ;
}


void	handle_poll_event(std::vector<pollfd>& pfds)
{
	if (pfds[0].revents & POLLIN){
		handle_server_connection(pfds);
	}
	for (long unsigned int i = 1; i <  pfds.size(); i++){
		if (!pfds[i].revents) continue ;
		if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
			close_error_socket(pfds, i);
			i--;
			continue ;
		}
		if (pfds[i].revents & POLLOUT) client_buffer_flush(pfds,i);
		if (pfds[i].revents & POLLIN) handle_client_input(pfds, i);

		// handle client request
	}
}

int	main(void){
	//signal(SIGNUM, action);
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
	int yes = 1;
	while (temp){
		sockfd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
		if (sockfd == -1){
			std::cerr << "Socket: error" << std::endl;
			temp = temp->ai_next;
			continue ;
		}
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));
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
	std::vector<pollfd> pfds;
	if (!update_poll_fds(pfds, sockfd, POLLIN)){
		std::cerr << "max fds limit reached! unable to start the server." << std::endl;
		return 1;
	}
	std::cout << "poll: server is listening to incomming connections." << std::endl;

	while (true)
	{
		int nevents = poll(&(pfds[0]), pfds.size(), 1000);
		if (nevents == -1){
			if (errno == EINTR) continue ;
			perror("poll");
			exit(1);
		}
		if (nevents){
			handle_poll_event(pfds);
		}
	}

	return 0;
}
