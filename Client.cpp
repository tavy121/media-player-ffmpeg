#include "Client.hpp"

#include <netdb.h>
#include <thread>
#include <cstring>

FILE *Client::file;
uint8_t Client::data[INBUF_SIZE];
std::list<uint8_t *> Client::dataList;

Client::Client()
{
}

Client::~Client()
{
}

void Client::initializeClient(const char *_host, const int &port)
{
   socketFd = socket(AF_INET, SOCK_DGRAM, 0);
   if (socketFd < 0)
   {
      fprintf(stderr, "Error opening the socket\n");
   }

   struct hostent *host;
   host = gethostbyname(_host);

   serverAddress.sin_family = AF_INET;
   serverAddress.sin_port = htons(port);
   serverAddress.sin_addr = *((struct in_addr *)host->h_addr);

   if (connect(socketFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
   {
      fprintf(stderr, "Error connecting\n");
   }

   std::string infos(clientName + " " + channel);

   file = fopen("primit.mp4", "wb");

   printf("Client ready\n");
}

void Client::manageClient()
{
   char aux[1024];

   while (true)
   {
      serverLen = sizeof(serverAddress);
      printf("\nSend GET\n");

      bzero(buffer, 256);

      //fgets(buffer,255,stdin);
      memcpy(buffer, "GET", strlen("GET") + 1);

      n = sendto(socketFd, buffer, 256, 0, (const sockaddr *)&serverAddress, serverLen);

      if (n < 0)
      {
         printf("Sendto");
      }

      std::thread t1(Client::receiveContinuously, socketFd, serverAddress, serverLen);
      if (t1.joinable())
      {
         t1.join();
      }
   }
}

void Client::receiveContinuously(int socketFd, struct sockaddr_in serverAddress, socklen_t serverLen)
{
   int n;
   uint8_t *myData;

   n = recvfrom(socketFd, data, INBUF_SIZE, 0, (struct sockaddr *)&serverAddress, &serverLen);
   if (n < 0)
   {
      printf("Error in infinite loop\n");
   }

   dataList.push_back(data);
   fwrite(data, sizeof(uint8_t), n, file);
   if(n < INBUF_SIZE)
   {
      fclose(file);
      exit(0);
   }
}
