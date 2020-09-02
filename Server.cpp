#include "Server.hpp"

#include <cstring>
#include <thread>
#include <chrono>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>


uint8_t Server::inbuf[INBUF_SIZE];
uint8_t *Server::data;
size_t Server::data_size;

Server::Server()
{
   memset(inbuf, 0, INBUF_SIZE);

}

Server::~Server()
{
   fclose(file);
   exit(0);
}

void Server::initializeServer(const int &port, const std::string &fileName)
{
   file = fopen(fileName.c_str(), "rb");
   if (!file)
   {
      fprintf(stderr, "Nu am putut deschide fisierul %s\n", fileName);
      exit(1);
   }

   portNumber = port;

   FD_ZERO(&readFds);
   FD_ZERO(&tempFds);

   socketFd = socket(AF_INET, SOCK_DGRAM, 0);
   if (socketFd < 0)
   {
      fprintf(stderr, "Error at opening the socket");
   }
   int dummy = 1;
   if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, (void *)&dummy, sizeof(dummy)) == -1)
   {
      fprintf(stderr, "Error at setting the socket");
   }

   memset((char *)&serverAdress, 0, sizeof(serverAdress));
   serverAdress.sin_family = AF_INET;
   serverAdress.sin_addr.s_addr = INADDR_ANY;
   serverAdress.sin_port = htons(portNumber);
   bzero(&(serverAdress.sin_zero), 8);

   if (bind(socketFd, (struct sockaddr *)&serverAdress, sizeof(struct sockaddr)) < 0)
   {
      fprintf(stderr, "Error at binding");
   }

   FD_SET(socketFd, &readFds);

   fprintf(stderr, "Server initialized...");
}

void Server::manageRuntime()
{
   while (!feof(file))
   {
      clientLen = sizeof(clientAdress);
      n = recvfrom(socketFd, buffer, 1024, 0, (struct sockaddr *)&clientAdress, &clientLen);
      if (n < 0)
      {
         printf("recvfrom");
      }
      write(1, "\nReceived a datagram: ", 21);
      write(1, buffer, n);

      data_size = fread(inbuf, 1, INBUF_SIZE, file);
      printf("\nData size = %d", data_size);
      if (!data_size)
      {
         printf("!data_size\n");
         break;
      }
      data = inbuf;

      if (strcmp(buffer, "GET") == 0)
      {
         sendContinuously(socketFd, clientAdress, clientLen);
         printf("\nData sent\n");
      }
     
   }
}

void Server::sendContinuously(const int &socketFd, const struct sockaddr_in &clientAddress, const __socklen_t &clientLen)
{
   int n;
   n = sendto(socketFd, Server::inbuf, Server::data_size, 0, (struct sockaddr *)&clientAddress, clientLen);
   printf("\nN=%d\n", n);
   if (n < 0)
   {
      printf("Error in sending the message\n");
   }
}
