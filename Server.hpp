#pragma once

#include "IServer.hpp"

#include <fstream>
#include <vector>
#include <map>

#define INBUF_SIZE 4 * 1024

struct ClientInfo
{
   std::string clientName;
   int16_t channel;
};

class Server : public IServer
{
public:
   Server();
   ~Server();

   void initializeServer(const int &port, const std::string &fileName) override;
   void manageRuntime() override;
   static void sendContinuously(const int &socketFd, const struct sockaddr_in &clientAddress, const __socklen_t &clientLen);

private:
   FILE *file;
   //const char *fileName = "test2.mov";
   static uint8_t inbuf[INBUF_SIZE];
   static uint8_t *data;
   static size_t data_size;

   char buffer[256];
   int portNumber;

   int socketFd;
   int newSocketFd;
   __socklen_t clientLen;
   struct sockaddr_in serverAdress, clientAdress;
   ssize_t n;

   fd_set readFds;
   fd_set tempFds;
   int fdMax;

   std::vector<int> theClients;
   std::map<int, ClientInfo> activeClientSockets;
};
