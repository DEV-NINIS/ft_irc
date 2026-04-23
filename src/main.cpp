#include "Server.hpp"

#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>

static void signalHandler(int sig) {
	(void)sig;
	std::cout << "STOP SERVER" << std::endl;
	g_running = false;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	const int port = std::atoi(argv[1]);
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port" << std::endl;
		return 1;
	}

	const std::string password = argv[2];
	if (password.empty()) {
		std::cerr << "Password cannot be empty" << std::endl;
		return 1;
	}

	std::signal(SIGINT, signalHandler);
	#ifndef _WIN32
	std::signal(SIGQUIT, signalHandler);
	#endif

	try {
		Server server(port, password);
		server.init();
		server.run();
		server.cleanup();
	} catch (const std::exception& e) {
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
