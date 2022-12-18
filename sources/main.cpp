#include "Server.hpp"

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "Error: Listening port and connection password not provided" << std::endl
		          << "./ircserv <port> <password>" << std::endl;
		return 1;
	}

	std::cout << "Listening to port " << argv[1] << std::endl;
	try {
		Server irc;
		irc.initialize(argv[1], argv[2]);
		irc.waitForClients();
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
