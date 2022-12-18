#ifndef SERVER_HPP
#define SERVER_HPP

// C++ Libraries
#include <iostream>
#include <cstdlib>

// C Libraries
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class Server {

	private:
		int                   m_port;
		std::string           m_password;
		int                   m_server_fd;
		struct sockaddr_in    m_address;

	public:
		Server();
		~Server();

		void setPort(std::string port);
		void setPassword(std::string password);

		void setSocketOptions();
		void bindAddressToPort();
		void listenForConnections();
		void acceptRequests();

		void initialize(std::string port, std::string password);
		void waitForClients();

};

#endif
