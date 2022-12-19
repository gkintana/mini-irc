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

void Server::waitForClients() {
	// int address_size = sizeof(m_address),
	//     request_socket = accept(m_server_fd, (struct sockaddr*)&m_address, (socklen_t*)&address_size);
	// if (request_socket < 0) {
	// 	throw std::runtime_error("Error: Invalid connection encountered");
	// }
	// std::cout << "Received a connection from the Irssi Client" << std::endl;
	int request_socket;
	socklen_t addrlen;
	std::string buffer;

	struct pollfd serv = {m_server_fd, POLLIN, 0};
	m_poll_fd.push_back(serv);

	while (1) {
		if (poll(&m_poll_fd[0], m_poll_fd.size(), -1) == -1) {
			throw std::runtime_error("poll failure");
		}

		for (int i = 0; i < static_cast<int>(m_poll_fd.size()); i++) {
			if (m_poll_fd[i].revents && POLLIN) {
				if (m_poll_fd[i].fd == m_server_fd) {
					addrlen = sizeof(m_address);
					request_socket = accept(m_server_fd, (struct sockaddr*)&m_address, &addrlen);
					if (request_socket == -1) {
						throw std::runtime_error("accept failure");
					} else {
						struct pollfd client = {request_socket, POLLIN, 0};
						m_poll_fd.push_back(client);

						std::cout << "Connected!" << std::endl;
						//  printf("pollserver: new connection from %s on socket %d\n",
                        //     inet_ntop(remoteaddr.ss_family,
                        //         get_in_addr((struct sockaddr*)&remoteaddr),
                        //         remoteIP, INET6_ADDRSTRLEN),
                        //     newfd);
						std::cout << m_poll_fd.size() << std::endl;
						send(request_socket, "CAP * ACK multi-prefix\r\n", strlen("CAP * ACK multi-prefix\r\n"), 0);
						send(request_socket, "001 root :Welcome to the Internet Relay Network root\r\n", strlen("001 root :Welcome to the Internet Relay Network root\r\n"), 0);
					}
				} else {
					int bytes = recv(m_poll_fd[i].fd, &buffer, sizeof(buffer), 0);
					int sender = m_poll_fd[i].fd;

					if (bytes <= 0) {
						if (bytes == 0) {
							std::cout << "pollserver: socket " << sender << " hung up" << std::endl;
						} else {
							std::cerr << "recv failure" << std::endl;
						}

						close(m_poll_fd[i].fd);
						m_poll_fd.erase(m_poll_fd.begin() + i);
					} else {
						for (int j = 0; j < static_cast<int>(m_poll_fd.size()); j++) {
							int destination = m_poll_fd[j].fd;

							if (destination != m_server_fd && destination != sender) {
								if (send(destination, &buffer, bytes, 0) == -1) {
									throw std::runtime_error("send failure");
								}
							}
						}
					}
				}
			}
		}
	}

}

void Server::initServer(std::string port, std::string password) {
	this->setPort(port);
	this->setPassword(password);
	this->setServerSocket(port);
}
