#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#ifdef _WIN32
#include <winsock2.h>
typedef SOCKET socket_t;
#else
typedef int socket_t;
#endif

class Client {
private:
	socket_t _fd;
	std::string _recvBuffer;
	std::string _sendBuffer;
	bool _authenticated;

public:
	Client();
	explicit Client(socket_t fd);

	socket_t getFd() const;
	bool isAuthenticated() const;
	void setAuthenticated(bool value);

	std::string& recvBuffer();
	std::string& sendBuffer();
};

#endif
