#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/ip.h>

class IServer
{
public:
	virtual void initializeServer(const int &port, const std::string &fileName) = 0;
	virtual void manageRuntime() = 0;
	static void sendContinuously(const int &socketFd, const struct sockaddr_in &clientAddress, const __socklen_t &clientLen);
};