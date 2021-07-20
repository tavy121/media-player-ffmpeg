#include "ActualPlayer.hpp"

#include <netdb.h>
#include <cassert>
#include <utility>
#include <chrono>
#include <list>
#include <algorithm>

#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "SDL2/SDL_thread.h"
#include "SDL2/SDL_syswm.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_audio.h"

extern "C"
{

#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>

#include <libavformat/avio.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}

#define SDL_AUDIO_BUFFER_SIZE 1024;

const size_t ActualPlayer::mQueueSize{2};

struct SwrContext *swrCtx = nullptr;
AVFrame wanted_frame;
AudioPacket gAudioQueue;

void audio_callback(void *userdata, Uint8 *stream, int len);

ActualPlayer::ActualPlayer(const std::string &fileName)
{
   mVideoDecoder = std::make_shared<Decoder>(fileName);

   mConverter = std::make_shared<Converter>(mVideoDecoder->getWidth(),
                                            mVideoDecoder->getHeight(),
                                            mVideoDecoder->getPixelFormat(),
                                            AV_PIX_FMT_YUV420P);
   //printf("Video width = %d, height= %d", mVideoDecoder->getWidth(),  mVideoDecoder->getHeight());

   // mUserInterface = std::make_unique<UserInterface>(mVideoDecoder->getWidth(),
   //                                                  mVideoDecoder->getHeight());
   mUserInterface = std::make_unique<UserInterface>(fileName, 1280, 720);

   mTimer = std::make_shared<Timer>();
   mVideoPacketQueue = std::make_unique<PacketQueue>(mQueueSize);
   mVideoFrameQueue = std::make_unique<FrameQueue>(mQueueSize);

   for (unsigned int i = 0; i < mVideoDecoder->getAvFormatContext()->nb_streams; i++)
   {
      if (mVideoDecoder->getAvFormatContext()->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
      {
         audioStream = i;
      }
   }

   if (audioStream != -1)
   {
      pCodecAudioParameters = mVideoDecoder->getAvFormatContext()->streams[audioStream]->codecpar;
   }
}

ActualPlayer::~ActualPlayer()
{
}

int ActualPlayer::findAudioCodec()
{
   //printf("ActualPlayer::%s start\n", __func__);

   int res = -1;
   pAudioCodec = avcodec_find_decoder(pCodecAudioParameters->codec_id);

   if (pAudioCodec == nullptr)
   {
      printf(" bad audio codec\n");
      return res;
   }

   pCodecAudioCtx = avcodec_alloc_context3(pAudioCodec);
   if (pCodecAudioCtx == nullptr)
   {
      printf(" bad audio codec\n");
      exit(-1);
   }

   res = avcodec_parameters_to_context(pCodecAudioCtx, pCodecAudioParameters);

   if (res < 0)
   {
      printf("Failed to get audio codec\n");

      avcodec_free_context(&pCodecAudioCtx);
      exit(-1);
   }
   res = avcodec_open2(pCodecAudioCtx, pAudioCodec, nullptr);

   if (res < 0)
   {
      printf("Failed to open audio codec\n");
      exit(-1);
   }

   //printf("ActualPlayer::%s end\n", __func__);
   return 1;
}

int ActualPlayer::allocMemory()
{
   //printf("ActualPlayer::%s start\n", __func__);
   swrCtx = swr_alloc();
   if (swrCtx == nullptr)
   {
      printf("Failed to load audio\n");
      exit(-1);
   }

   //audio context
   av_opt_set_channel_layout(swrCtx, "in_channel_layout", pCodecAudioCtx->channel_layout, 0);
   av_opt_set_channel_layout(swrCtx, "out_channel_layout", pCodecAudioCtx->channel_layout, 0);
   av_opt_set_int(swrCtx, "in_sample_rate", pCodecAudioCtx->sample_rate, 0);
   av_opt_set_int(swrCtx, "out_sample_rate", pCodecAudioCtx->sample_rate, 0);
   av_opt_set_sample_fmt(swrCtx, "in_sample_fmt", pCodecAudioCtx->sample_fmt, 0);
   av_opt_set_sample_fmt(swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);

   int res = swr_init(swrCtx);

   if (res != 0)
   {
      printf("Failed to initialize audio\n");
      exit(-1);
   }

   memset(&wantedSpec, 0, sizeof(wantedSpec));
   wantedSpec.channels = pCodecAudioCtx->channels;
   wantedSpec.freq = pCodecAudioCtx->sample_rate;
   wantedSpec.format = AUDIO_S16SYS;
   wantedSpec.silence = 0;
   wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
   wantedSpec.userdata = pCodecAudioCtx;
   wantedSpec.callback = audio_callback;

   if (SDL_OpenAudio(&wantedSpec, &audioSpec) < 0)
   {
      printf("Error opening audio\n");
      exit(-1);
   }
   wanted_frame.format = AV_SAMPLE_FMT_S16;
   wanted_frame.sample_rate = audioSpec.freq;
   wanted_frame.channel_layout = av_get_default_channel_layout(audioSpec.channels);
   wanted_frame.channels = audioSpec.channels;

   initAudioPacket(&gAudioQueue);
   SDL_PauseAudio(0);

   //printf("ActualPlayer::%s end\n", __func__);
   return 1;
}

void ActualPlayer::initAudioPacket(AudioPacket *audioQueue)
{
   //printf("ActualPlayer::%s start\n", __func__);
   audioQueue->last = nullptr;
   audioQueue->first = nullptr;
   audioQueue->mutex = SDL_CreateMutex();
   audioQueue->cond = SDL_CreateCond();
   //printf("ActualPlayer::%s end\n", __func__);
}

int ActualPlayer::putAudioPacket(AudioPacket *audioQueue, AVPacket *pkt)
{
   //printf("ActualPlayer::%s start\n", __func__);
   AVPacketList *pktl;
   AVPacket *newPkt;
   newPkt = (AVPacket *)av_mallocz_array(1, sizeof(AVPacket));
   if (av_packet_ref(newPkt, pkt) < 0)
   {
      return -1;
   }

   pktl = (AVPacketList *)av_malloc(sizeof(AVPacketList));
   if (!pktl)
   {
      return -1;
   }

   pktl->pkt = *newPkt;
   pktl->next = nullptr;

   SDL_LockMutex(audioQueue->mutex);

   if (!audioQueue->last)
      audioQueue->first = pktl;
   else
      audioQueue->last->next = pktl;

   audioQueue->last = pktl;

   audioQueue->nb_packets++;
   audioQueue->size += newPkt->size;

   SDL_CondSignal(audioQueue->cond);
   SDL_UnlockMutex(audioQueue->mutex);

   //printf("ActualPlayer::%s end\n", __func__);
   return 0;
}

int ActualPlayer::getAudioPacket(AudioPacket *audioQueue, AVPacket *pkt, int block)
{
   //printf("ActualPlayer::%s start\n", __func__);
   AVPacketList *pktl;
   int ret;

   SDL_LockMutex(audioQueue->mutex);

   while (1)
   {
      pktl = audioQueue->first;
      if (pktl)
      {
         audioQueue->first = pktl->next;
         if (!audioQueue->first)
            audioQueue->last = nullptr;

         audioQueue->nb_packets--;
         audioQueue->size -= pktl->pkt.size;

         *pkt = pktl->pkt;
         av_free(pktl);
         ret = 1;
         break;
      }
      else if (!block)
      {
         ret = 0;
         break;
      }
      else
      {
         SDL_CondWait(audioQueue->cond, audioQueue->mutex);
      }
   }

   SDL_UnlockMutex(audioQueue->mutex);

   //printf("ActualPlayer::%s end\n", __func__);
   return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size)
{
   //printf("ActualPlayer::%s start\n", __func__);
   static AVPacket pkt;
   static uint8_t *audio_pkt_data = nullptr;
   static int audio_pkt_size = 0;
   static AVFrame frame;

   int len1;
   int data_size = 0;

   SwrContext *swr_ctx = nullptr;

   while (1)
   {
      while (audio_pkt_size > 0)
      {
         int got_frame = 0;

         avcodec_send_packet(aCodecCtx, &pkt);
         avcodec_receive_frame(aCodecCtx, &frame);

         len1 = frame.pkt_size;
         if (len1 < 0)
         {
            audio_pkt_size = 0;
            break;
         }

         audio_pkt_data += len1;
         audio_pkt_size -= len1;
         data_size = 0;
         if (got_frame)
         {
            int linesize = 1;
            data_size = av_samples_get_buffer_size(&linesize, aCodecCtx->channels, frame.nb_samples, aCodecCtx->sample_fmt, 1);
            assert(data_size <= buf_size);
            memcpy(audio_buf, frame.data[0], data_size);
         }

         if (frame.channels > 0 && frame.channel_layout == 0)
         {
            frame.channel_layout = av_get_default_channel_layout(frame.channels);
         }
         else if (frame.channels == 0 && frame.channel_layout > 0)
         {
            frame.channels = av_get_channel_layout_nb_channels(frame.channel_layout);
         }

         if (swr_ctx)
         {
            swr_free(&swr_ctx);
            swr_ctx = nullptr;
         }

         swr_ctx = swr_alloc_set_opts(nullptr, wanted_frame.channel_layout, (AVSampleFormat)wanted_frame.format, wanted_frame.sample_rate,
                                      frame.channel_layout, (AVSampleFormat)frame.format, frame.sample_rate, 0, nullptr);

         if (!swr_ctx || swr_init(swr_ctx) < 0)
         {
            printf("swr_init failed\n");
            break;
         }

         int dst_nb_samples = (int)av_rescale_rnd(swr_get_delay(swr_ctx, frame.sample_rate) + frame.nb_samples,
                                                  wanted_frame.sample_rate, wanted_frame.format, AV_ROUND_INF);
         int len2 = swr_convert(swr_ctx, &audio_buf, dst_nb_samples,
                                (const uint8_t **)frame.data, frame.nb_samples);
         if (len2 < 0)
         {
            printf("swr_convert failed\n");
            break;
         }

         return wanted_frame.channels * len2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

         if (data_size <= 0)
         {
            continue;
         }

         return data_size;
      }

      if (pkt.data)
         av_packet_unref(&pkt);

      if (ActualPlayer::getAudioPacket(&gAudioQueue, &pkt, 1) < 0)
      {
         return -1;
      }

      audio_pkt_data = pkt.data;
      audio_pkt_size = pkt.size;
   }
   //printf("ActualPlayer::%s end\n", __func__);
}

void audio_callback(void *userdata, Uint8 *stream, int len)
{
   AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
   int len1, audio_size;

   static uint8_t audio_buff[192000 * 3 / 2];
   static unsigned int audio_buf_size = 0;
   static unsigned int audio_buf_index = 0;

   SDL_memset(stream, 0, len);

   while (len > 0)
   {
      if (audio_buf_index >= audio_buf_size)
      {
         audio_size = audio_decode_frame(aCodecCtx, audio_buff, sizeof(audio_buff));
         if (audio_size < 0)
         {
            audio_buf_size = 1024;
            memset(audio_buff, 0, audio_buf_size);
         }
         else
         {
            audio_buf_size = audio_size;
         }

         audio_buf_index = 0;
      }
      len1 = audio_buf_size - audio_buf_index;
      if (len1 > len)
      {
         len1 = len;
      }

      SDL_MixAudio(stream, audio_buff + audio_buf_index, len, SDL_MIX_MAXVOLUME);

      //memcpy(stream, (uint8_t*)(audio_buff + audio_buf_index), audio_buf_size);
      len -= len1;
      stream += len1;
      audio_buf_index += len1;
   }
}

void ActualPlayer::operator()()
{
   mThreadPool.emplace_back(&ActualPlayer::demultiplex, this);
   mThreadPool.emplace_back(&ActualPlayer::decodeMedia, this);
   findAudioCodec();
   allocMemory();
   playVideo();

   for (auto &thread : mThreadPool)
   {
      thread.join();
   }

   if (mException)
   {
      std::rethrow_exception(mException);
   }
}

void ActualPlayer::memsetAudioPacket(AudioPacket *audioQueue)
{
}

void ActualPlayer::demultiplex()
{
   try
   {
      while (1)
      {
         std::unique_ptr<AVPacket, std::function<void(AVPacket *)>> packet{
             new AVPacket,
             [](AVPacket *p) { av_packet_unref(p); delete p; }};
         av_init_packet(packet.get());
         packet->data = nullptr;

         if (!mVideoDecoder->readAvFrame(*packet))
         {
            mVideoPacketQueue->finished();
            printf("I m finished\n");
            break;
         }

         if (packet->stream_index == mVideoDecoder->getVideoStreamIndex())
         {
            //printf("I ve entered in video stream index\n");
            if (!mVideoPacketQueue->push(std::move(packet)))
            {
               break;
            }
         }
         else if (packet->stream_index == audioStream)
         {
            putAudioPacket(&gAudioQueue, packet.get());
         }
      }
   }
   catch (...)
   {
      mException = std::current_exception();
      mVideoFrameQueue->quit();
      mVideoPacketQueue->quit();
   }
}

void ActualPlayer::decodeMedia()
{
   try
   {
      const AVRational microseconds = {1, 1000000};
      while (1)
      {
         std::unique_ptr<AVFrame, std::function<void(AVFrame *)>>
             decodedFrame{
                 av_frame_alloc(), [](AVFrame *f) { av_frame_free(&f); }};

         std::unique_ptr<AVPacket, std::function<void(AVPacket *)>> packet{
             nullptr, [](AVPacket *p) { av_packet_unref(p); delete p; }};

         if (!mVideoPacketQueue->pop(packet))
         {
            mVideoFrameQueue->finished();
            break;
         }

         bool sent = false;

         while (!sent)
         {
            sent = mVideoDecoder->send(packet.get());

            while (mVideoDecoder->receive(decodedFrame.get()))
            {
               decodedFrame->pts = av_rescale_q(decodedFrame->pkt_dts, mVideoDecoder->getFormatTimeBase(), microseconds);

               std::unique_ptr<AVFrame, std::function<void(AVFrame *)>>
                   convertedFrame{
                       av_frame_alloc(),
                       [](AVFrame *f) { av_free(f->data[0]); }};

               if (av_frame_copy_props(convertedFrame.get(), decodedFrame.get()) < 0)
               {
                  throw std::runtime_error("Copying frame props");
               }

               if (av_image_alloc(convertedFrame->data,
                                  convertedFrame->linesize,
                                  mVideoDecoder->getWidth(),
                                  mVideoDecoder->getHeight(),
                                  mVideoDecoder->getPixelFormat(),
                                  1) < 0)
               {
                  throw std::runtime_error("Allocating picture");
               }

               (*mConverter)(decodedFrame.get(), convertedFrame.get());

               if (!mVideoFrameQueue->push(std::move(convertedFrame)))
               {
                  break;
               }
            }
         }
      }
   }
   catch (...)
   {
      mException = std::current_exception();
      mVideoFrameQueue->quit();
      mVideoPacketQueue->quit();
   }
}

void ActualPlayer::playVideo()
{
   try
   {
      int64_t lastPts = 0;

      for (size_t frameNumber = 0;; ++frameNumber)
      {
         mUserInterface->input();

         if (mUserInterface->getQuit())
         {
            break;
         }
         else if (mUserInterface->getPlay())
         {
            std::unique_ptr<AVFrame, std::function<void(AVFrame *)>> frame{
                nullptr, [](AVFrame *f) { av_frame_free(&f); }};

            if (!mVideoFrameQueue->pop(frame))
            {
               break;
            }

            if (frameNumber)
            {
               const int64_t frameDelay = frame->pts - lastPts;
               lastPts = frame->pts;
               mTimer->wait(frameDelay);
            }
            else
            {
               lastPts = frame->pts;
               mTimer->update();
            }

            mUserInterface->refreshVideo(
                {frame->data[0], frame->data[1], frame->data[2]},
                {static_cast<size_t>(frame->linesize[0]),
                 static_cast<size_t>(frame->linesize[1]),
                 static_cast<size_t>(frame->linesize[2])});
         }
         else
         {
            std::chrono::milliseconds sleep(10);
            std::this_thread::sleep_for(sleep);
            mTimer->update();
         }
      }
   }
   catch (...)
   {
      mException = std::current_exception();
   }

   mVideoFrameQueue->quit();
   mVideoPacketQueue->quit();
}
