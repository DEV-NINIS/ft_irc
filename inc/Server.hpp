#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <vector>

#include "Client.hpp"

#ifdef _WIN32
#include <winsock2.h>
typedef WSAPOLLFD pollfd_t;
#else
#include <poll.h>
typedef struct pollfd pollfd_t;
#endif

extern bool g_running;

class Server {
private:
	int _port;
	std::string _password;
	socket_t _sockfd;
	std::vector<pollfd_t> _pollFds;
	std::map<socket_t, Client> _clients;

	void enablePollout(socket_t fd);

public:
	Server(int port, const std::string& password);
	~Server();

	void init();
	void run();
	void acceptClient();
	void receiveClient(socket_t fd);
	void disconnectClient(socket_t fd);
	void sendClient(socket_t fd);
	void cleanup();
};

#endif
