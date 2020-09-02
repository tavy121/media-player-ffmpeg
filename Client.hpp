#pragma once

#include "IClient.hpp"
#include <list>
#include <fstream>
#include <string>

#define INBUF_SIZE 4 * 1024

class Client : public IClient
{
public:
   Client();
   ~Client();

   void initializeClient(const char *_host, const int &port) override;
   void manageClient() override;
   static void receiveContinuously(int socketFd, struct sockaddr_in serverAddress, socklen_t serverLen);
   static void consumerThread();

private:
   static FILE *file;

   //data part
   static std::list<uint8_t *> dataList;
   static uint8_t data[INBUF_SIZE];
   char buffer[256];

   std::string clientName;
   std::string channel;

   //socket part
   int socketFd;
   ssize_t n;
   struct sockaddr_in serverAddress;
   socklen_t serverLen;

   fd_set readFds;
   fd_set tempFds;
   int fdMax;

};