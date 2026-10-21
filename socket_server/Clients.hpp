#pragma once
#include <ctime>
#include <iostream>
#include <sstream>

class Client {
	private:
	public:
		Client();
		~Client();
		Client(const Client&);
		const Client& operator=(const Client&);
		std::string	name;
		bool		registered;		
};
