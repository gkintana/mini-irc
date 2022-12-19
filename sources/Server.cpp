#include "Server.hpp"

Server::Server() {}

Server::~Server() {
	close(m_server_fd);
}

void Server::setPort(std::string port) {
	this->m_port = std::atoi(port.c_str());
}

void Server::setPassword(std::string password) {
	this->m_password = password;
}

struct addrinfo* Server::getAddresses(std::string port) {
	struct addrinfo hints = {0, 0, 0, 0, 0, 0, 0, 0},
	                        *address_structs = 0;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(0, port.c_str(), &hints, &address_structs) != 0) {
		throw std::runtime_error("getaddrinfo");
	}
	return address_structs;
}

void Server::setServerSocket(std::string port) {
	struct addrinfo *address_structs = this->getAddresses(port);
	this->socketLoop(address_structs);
	this->listenForConnections();
}

int Server::createSocket(struct addrinfo *ptr) {
	return m_server_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
}

int Server::bindToAddress(struct addrinfo* ptr) {
	return bind(m_server_fd, ptr->ai_addr, ptr->ai_addrlen);
}

void Server::checkSocket(struct addrinfo *address_structs, struct addrinfo *ptr) {
	freeaddrinfo(address_structs);
	if (!ptr) {
		throw std::runtime_error("Error: Binding of address to socket failed");
	}
}

void Server::socketLoop(struct addrinfo* address_structs) {
	int socket_option = 1;
	struct addrinfo *ptr;

	for (ptr = address_structs; ptr != NULL; ptr = ptr->ai_next) {
		if (this->createSocket(ptr) == -1) {
			continue;
		}
		setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option));

		if (this->bindToAddress(ptr) == -1) {
			close(m_server_fd);
			continue;
		}
		break;
	}
	this->checkSocket(address_structs, ptr);
}

void Server::listenForConnections() {
	if (listen(m_server_fd, 10) == -1) {
		throw std::runtime_error("Error: Unable to listen to connections");
	}
}

void Server::doPoll() {
	if (poll(&m_poll_fd[0], m_poll_fd.size(), -1) == -1) {
		throw std::runtime_error("Error: poll failure");
	}
}

void Server::removeClient(int bytes, int src_fd, int i) {
	if (!bytes) {
		std::cout << "Server: socket " << src_fd << " hung up" << std::endl;
	} else {
		std::cerr << "Error: recv failure" << std::endl;
	}
	close(m_poll_fd[i].fd);
	m_poll_fd.erase(m_poll_fd.begin() + i);
}

void Server::sendMessage(int bytes, int dest_fd, const char* buffer) {
	if (send(dest_fd, buffer, bytes, 0) == -1) {
		throw std::runtime_error("send failure");
	}
}

void Server::sendToAllClients(int bytes, int src_fd, const char* buffer) {
	for (int i = 0; i < static_cast<int>(m_poll_fd.size()); i++) {
		int dest_fd = m_poll_fd[i].fd;

		if (dest_fd != m_server_fd && dest_fd != src_fd) {
			this->sendMessage(bytes, dest_fd, buffer);
		}
	}
}

int Server::acceptConnections() {
	int status = 0;
	socklen_t addrlen = sizeof(m_address);

	return status = accept(m_server_fd, (struct sockaddr*)&m_address, &addrlen);
}

void Server::addClient(int client_socket) {
	struct pollfd client = {client_socket, POLLIN, 0};
	m_poll_fd.push_back(client);

	//  printf("pollserver: new connection from %s on socket %d\n",
	//     inet_ntop(remoteaddr.ss_family,
	//         get_in_addr((struct sockaddr*)&remoteaddr),
	//         remoteIP, INET6_ADDRSTRLEN),
	//     newfd);
	std::cout << "Connected!\nTotal Users: " << m_poll_fd.size() - 1 << std::endl;
	this->doCapabilityNegotiation(client_socket);
}

void Server::doCapabilityNegotiation(int client_socket) {
	send(client_socket, "CAP * ACK multi-prefix\r\n", strlen("CAP * ACK multi-prefix\r\n"), 0);
	send(client_socket, "001 root :Welcome to the Internet Relay Network root\r\n", strlen("001 root :Welcome to the Internet Relay Network root\r\n"), 0);
}

void Server::handleNewConnections() {
	int client_socket = this->acceptConnections();
	if (client_socket == -1) {
		throw std::runtime_error("Error: Encountered invalid connection");
	} else {
		this->addClient(client_socket);
	}
}

void Server::manageConnections(int i) {
	char buffer[1024];
	int bytes = recv(m_poll_fd[i].fd, buffer, sizeof(buffer), 0),
	    src_fd = m_poll_fd[i].fd;

	bytes <= 0 ? this->removeClient(bytes, src_fd, i) :
	             this->sendToAllClients(bytes, src_fd, buffer);
}

void Server::waitForClients() {
	struct pollfd serv = {m_server_fd, POLLIN, 0};
	m_poll_fd.push_back(serv);

	while (1) {
		this->doPoll();
		for (int i = 0; i < static_cast<int>(m_poll_fd.size()) && (m_poll_fd[i].revents && POLLIN); i++) {
			m_poll_fd[i].fd == m_server_fd ? this->handleNewConnections() : this->manageConnections(i);
		}
	}
}

void Server::initServer(std::string port, std::string password) {
	this->setPort(port);
	this->setPassword(password);
	this->setServerSocket(port);
}
