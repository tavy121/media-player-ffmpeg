#pragma once

#include "SDL2/SDL.h"

extern "C"
{
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
}

typedef struct _AudioPacket
{
   AVPacketList *first, *last;
   int nb_packets;
   int size;
   SDL_mutex *mutex;
   SDL_cond *cond;
} AudioPacket;

class IActualPlayer
{
public:

   virtual void demultiplex() = 0;
   virtual void decodeMedia() = 0;
   virtual void playVideo() = 0;
   virtual void operator()() = 0;

   virtual void memsetAudioPacket(AudioPacket *audioPacket) = 0;

   virtual void initAudioPacket(AudioPacket *) = 0;

   virtual int putAudioPacket(AudioPacket *audioQueue, AVPacket *packet) = 0;
   virtual int findAudioCodec() = 0;
   virtual int allocMemory() = 0;

   static int getAudioPacket(AudioPacket *audioQueue, AVPacket *packet, int block);

   //Client
   // void initializeClient(int argc, char *argv[]);
   // void runClient();
   // void receiveMediaData(int socketFd, struct sockaddr_in serverAddress, socklen_t serverLen);
   // void consumeVideoData();

};