#include "Server.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {
#ifdef _WIN32
const short kReadEvent = POLLRDNORM;
const short kWriteEvent = POLLWRNORM;

socket_t invalidSocket() {
	return INVALID_SOCKET;
}

int portablePoll(pollfd_t* fds, unsigned long count, int timeout) {
	return WSAPoll(fds, count, timeout);
}

int closeSocket(socket_t fd) {
	return closesocket(fd);
}

bool wouldBlock() {
	const int err = WSAGetLastError();
	return err == WSAEWOULDBLOCK;
}

bool setNonBlocking(socket_t fd) {
	u_long mode = 1;
	return ioctlsocket(fd, FIONBIO, &mode) == 0;
}
#else
const short kReadEvent = POLLIN;
const short kWriteEvent = POLLOUT;

socket_t invalidSocket() {
	return -1;
}

int portablePoll(pollfd_t* fds, unsigned long count, int timeout) {
	return poll(fds, count, timeout);
}

int closeSocket(socket_t fd) {
	return close(fd);
}

bool wouldBlock() {
	return errno == EAGAIN || errno == EWOULDBLOCK;
}

bool setNonBlocking(socket_t fd) {
	return fcntl(fd, F_SETFL, O_NONBLOCK) >= 0;
}
#endif
}

bool g_running = true;

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _sockfd(invalidSocket()) {}

Server::~Server() {
	cleanup();
}

void Server::init() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd == invalidSocket()) {
		std::cerr << "Error socket" << std::endl;
		throw std::runtime_error("socket failed");
	}

	int opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0) {
		std::cerr << "Error setsockopt" << std::endl;
		throw std::runtime_error("setsockopt failed");
	}

	sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

	if (bind(_sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
		std::cerr << "Error bind" << std::endl;
		throw std::runtime_error("bind failed");
	}

	if (listen(_sockfd, SOMAXCONN) < 0) {
		std::cerr << "Error listen" << std::endl;
		throw std::runtime_error("listen failed");
	}

	if (!setNonBlocking(_sockfd)) {
		std::cerr << "Error fcntl" << std::endl;
		throw std::runtime_error("fcntl failed");
	}

	pollfd_t serverPoll;
	serverPoll.fd = _sockfd;
	serverPoll.events = kReadEvent;
	serverPoll.revents = 0;
	_pollFds.push_back(serverPoll);

	std::cout << "Server listening on port " << _port << std::endl;
}

void Server::run() {
	while (g_running) {
		int rc = portablePoll(&_pollFds[0], static_cast<unsigned long>(_pollFds.size()), -1);
		if (rc < 0) {
		#ifndef _WIN32
			if (errno == EINTR) {
				continue;
			}
		#endif
			std::cerr << "Error poll" << std::endl;
			break;
		}

		for (size_t i = 0; i < _pollFds.size();) {
			const socket_t fd = _pollFds[i].fd;
			const short revents = _pollFds[i].revents;
			bool removed = false;

			if (revents == 0) {
				++i;
				continue;
			}

			if (fd == _sockfd) {
				if (revents & kReadEvent) {
					acceptClient();
				}
				++i;
				continue;
			}

			if (revents & (POLLHUP | POLLERR | POLLNVAL)) {
				disconnectClient(fd);
				removed = true;
			}

			if (!removed && (revents & kReadEvent)) {
				receiveClient(fd);
				if (_clients.find(fd) == _clients.end()) {
					removed = true;
				}
			}

			if (!removed && (revents & kWriteEvent)) {
				sendClient(fd);
				if (_clients.find(fd) == _clients.end()) {
					removed = true;
				}
			}

			if (!removed) {
				++i;
			}
		}
	}
}

void Server::acceptClient() {
	socket_t clientFd = accept(_sockfd, NULL, NULL);
	if (clientFd == invalidSocket()) {
		return;
	}

	if (!setNonBlocking(clientFd)) {
		std::cerr << "Error fcntl" << std::endl;
		closeSocket(clientFd);
		return;
	}

	pollfd_t pfd;
	pfd.fd = clientFd;
	pfd.events = kReadEvent;
	pfd.revents = 0;

	_pollFds.push_back(pfd);
	_clients[clientFd] = Client(clientFd);

	std::cout << "Client connected (fd=" << clientFd << "), total clients=" << _clients.size() << std::endl;
}

void Server::receiveClient(socket_t fd) {
	char buffer[1024];
	const int bytes = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes == 0) {
		disconnectClient(fd);
		return;
	}

	if (bytes < 0) {
		if (!wouldBlock()) {
			disconnectClient(fd);
		}
		return;
	}

	std::string& recvBuffer = _clients[fd].recvBuffer();
	recvBuffer.append(buffer, bytes);

	size_t pos = std::string::npos;
	while ((pos = recvBuffer.find("\r\n")) != std::string::npos) {
		std::string line = recvBuffer.substr(0, pos);
		recvBuffer.erase(0, pos + 2);

		std::cout << "Received: " << line << std::endl;

		// Hook parser/commands here.
		_clients[fd].sendBuffer() += ":ft_irc NOTICE * :Message received\r\n";
		enablePollout(fd);
	}
}

void Server::disconnectClient(socket_t fd) {
	std::cout << "Client disconnected: " << fd << std::endl;

	closeSocket(fd);

	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds.erase(_pollFds.begin() + static_cast<std::vector<pollfd>::difference_type>(i));
			break;
		}
	}

	_clients.erase(fd);
}

void Server::sendClient(socket_t fd) {
	std::string& sendBuffer = _clients[fd].sendBuffer();
	if (sendBuffer.empty()) {
		return;
	}

	const int bytes = send(fd, sendBuffer.c_str(), sendBuffer.size(), 0);
	if (bytes < 0) {
		if (!wouldBlock()) {
			disconnectClient(fd);
		}
		return;
	}

	sendBuffer.erase(0, bytes);
	if (sendBuffer.empty()) {
		for (size_t i = 0; i < _pollFds.size(); ++i) {
			if (_pollFds[i].fd == fd) {
				_pollFds[i].events &= static_cast<short>(~kWriteEvent);
				break;
			}
		}
	}
}

void Server::enablePollout(socket_t fd) {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		if (_pollFds[i].fd == fd) {
			_pollFds[i].events |= kWriteEvent;
			return;
		}
	}
}

void Server::cleanup() {
	for (size_t i = 0; i < _pollFds.size(); ++i) {
		closeSocket(_pollFds[i].fd);
	}
	_pollFds.clear();
	_clients.clear();
	_sockfd = invalidSocket();

#ifdef _WIN32
    WSACleanup();
#endif
}
