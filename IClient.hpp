#pragma once

#include <netinet/in.h>

class IClient
{
public:
   virtual void initializeClient(const char *_host, const int &port) = 0;
   virtual void manageClient() = 0;
   static void receiveContinuously(int socketFd, struct sockaddr_in serverAddress, socklen_t serverLen);
   static void consumerThread();
};