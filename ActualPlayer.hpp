#pragma once

#include "IActualPlayer.hpp"
#include "Decoder.hpp"
#include "Utils.hpp"
#include "UserInterface.hpp"
#include "CustomQueue.hpp"

#include <string>
#include <memory>
#include <thread>

class ActualPlayer : public IActualPlayer
{
public:
   ActualPlayer(const std::string &fileName);
   ~ActualPlayer();

   void demultiplex() override;
   void decodeMedia() override;
   void playVideo() override;
   void operator()() override;

   void memsetAudioPacket(AudioPacket *audioQueue) override;

   void initAudioPacket(AudioPacket *audioQueue) override;

   int putAudioPacket(AudioPacket *audioQueue, AVPacket *) override;
   int findAudioCodec() override;
   int allocMemory() override;

   static int getAudioPacket(AudioPacket *audioQueue, AVPacket *packet, int block);

private:
   //PlayerPart
   std::shared_ptr<Decoder> mVideoDecoder;
   std::shared_ptr<Converter> mConverter;
   std::unique_ptr<UserInterface> mUserInterface;
   std::shared_ptr<Timer> mTimer;
   std::unique_ptr<PacketQueue> mVideoPacketQueue;
   std::unique_ptr<FrameQueue> mVideoFrameQueue;
   std::vector<std::thread> mThreadPool;

   static const size_t mQueueSize;
   std::exception_ptr mException{};

   SDL_AudioSpec wantedSpec = {0};
   SDL_AudioSpec audioSpec = {0};

   AVCodecParameters *pCodecAudioParameters = nullptr;
   AVCodecContext *pCodecAudioCtx = nullptr;
   AVCodec *pAudioCodec = nullptr;
   int audioStream;

};
