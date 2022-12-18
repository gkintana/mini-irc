#include "Server.hpp"

Server::Server() {
	if ((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		throw std::runtime_error("Error: Server socket creation failed");
	}
}

Server::~Server() {
	close(m_server_fd);
}

void Server::setPort(std::string port) {
	this->m_port = std::atoi(port.c_str());
}

void Server::setPassword(std::string password) {
	this->m_password = password;
}

void Server::setSocketOptions() {
	int option = 1;
	if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
		throw std::runtime_error("Error: Manipulation of socket options failed");
	}
	m_address.sin_family = AF_INET;
	m_address.sin_addr.s_addr = INADDR_ANY;
	m_address.sin_port = htons(m_port);
}

void Server::bindAddressToPort() {
	if (bind(m_server_fd, (struct sockaddr*)&m_address, sizeof(m_address)) < 0) {
		throw std::runtime_error("Error: Binding of address to port failed");
	}
}

void Server::listenForConnections() {
	if (listen(m_server_fd, 3) < 0) {
		throw std::runtime_error("Error: Unable to listen to connections");
	}
}

void Server::acceptRequests() {
	int address_size = sizeof(m_address),
	    request_socket = accept(m_server_fd, (struct sockaddr*)&m_address, (socklen_t*)&address_size);
	if (request_socket < 0) {
		throw std::runtime_error("Error: Invalid connection encountered");
	}
	std::cout << "Received a connection from the Irssi Client" << std::endl;
}

void Server::initialize(std::string port, std::string password) {
	this->setPort(port);
	this->setPassword(password);
	this->setSocketOptions();
	this->bindAddressToPort();
}

void Server::waitForClients() {
	this->listenForConnections();
	this->acceptRequests();
}
