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

		void doPoll();
		void handleNewConnections();
		void manageConnections(int i);
		int acceptConnections();
		void doCapabilityNegotiation(int client_socket);
		void addClient(int client_socket);
		void removeClient(int bytes, int src_fd, int i);
		void sendMessage(int bytes, int dest_fd, const char* buffer);
		void sendToAllClients(int bytes, int src_fd, const char* buffer);

	public:
		Server();
		~Server();

		void initServer(std::string port, std::string password);
		void waitForClients();

};

#endif
