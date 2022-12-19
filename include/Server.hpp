#ifndef SERVER_HPP
#define SERVER_HPP

// C++ Libraries
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>

// C Libraries
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>

class Server {

	private:
		int                           m_port;
		std::string                   m_password;
		int                           m_server_fd;
		struct sockaddr_storage       m_address;
		std::vector<struct pollfd>    m_poll_fd;

		void setPort(std::string port);
		void setPassword(std::string password);
		void setServerSocket(std::string port);

		struct addrinfo* getAddresses(std::string port);
		int createSocket(struct addrinfo *ptr);
		int bindToAddress(struct addrinfo* ptr);
		void checkSocket(struct addrinfo* address_structs, struct addrinfo *ptr);
		void socketLoop(struct addrinfo* address_structs);
		void listenForConnections();

	public:
		Server();
		~Server();

		void initServer(std::string port, std::string password);
		void waitForClients();

};

#endif
