#pragma once
#include <ctime>
#include <iostream>
#include <sstream>
#include "server.hpp"

class Client {
	private:
	public:
		Client();
		~Client();
		Client(const Client&);
		const Client& operator=(const Client&);
		std::string	name;
		bool		registered;		
		std::string	buffer_out; // being sent 
		std::string	buffer_in; // being recieved
};
