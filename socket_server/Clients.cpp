#include "Clients.hpp"

Client::Client(): name(""), registered(false){
	std::ostringstream os;
	os << "Anony" << std::time(0); 
	name = os.str();
}
Client::Client(const Client& copy)
{
	if (this != &copy)
	{
		this->name = copy.name;
		this->registered = copy.registered;
	}
}
Client::~Client(){
}
const Client&	Client::operator=(const Client& copy){

	if (this != &copy)
	{
		this->name = copy.name;
		this->registered = copy.registered;
	}
	return (*this);
}
