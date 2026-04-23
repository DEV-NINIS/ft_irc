#include "Client.hpp"

#ifdef _WIN32
Client::Client() : _fd(INVALID_SOCKET), _authenticated(false) {}
#else
Client::Client() : _fd(-1), _authenticated(false) {}
#endif

Client::Client(socket_t fd) : _fd(fd), _authenticated(false) {}

socket_t Client::getFd() const {
	return _fd;
}

bool Client::isAuthenticated() const {
	return _authenticated;
}

void Client::setAuthenticated(bool value) {
	_authenticated = value;
}

std::string& Client::recvBuffer() {
	return _recvBuffer;
}

std::string& Client::sendBuffer() {
	return _sendBuffer;
}
