#include "Decoder.hpp"

//C includes
#include <cstdio>

Decoder::Decoder(const std::string &fileName)
{
   av_register_all();
   avcodec_register_all();
   
   DecoderUtils::checkFFMPEG(avformat_open_input(&mAvFormatContext, fileName.c_str(), nullptr, nullptr));
   DecoderUtils::checkFFMPEG(avformat_find_stream_info(mAvFormatContext, nullptr));

   mVideoStreamIndex = DecoderUtils::checkFFMPEG(av_find_best_stream(
       mAvFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0));
   mAudioStremIndex = DecoderUtils::checkFFMPEG(av_find_best_stream(
       mAvFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0));

   const auto videoCodecParameters = mAvFormatContext->streams[mVideoStreamIndex]->codecpar;
   const auto audioCodecParameters = mAvFormatContext->streams[mAudioStremIndex]->codecpar;

   const auto videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
   if (!videoCodec)
   {
      printf("Codec wasn't found\n");
      throw new error_t();
   }
   mAvCodecContext = avcodec_alloc_context3(videoCodec);
   if (!mAvCodecContext)
   {
      printf("Could't alloc context\n");
      throw new error_t();
   }
   DecoderUtils::checkFFMPEG(avcodec_parameters_to_context(mAvCodecContext, videoCodecParameters));
   DecoderUtils::checkFFMPEG(avcodec_open2(mAvCodecContext, videoCodec, nullptr));
}

Decoder::~Decoder()
{
   avcodec_free_context(&mAvCodecContext);
   avformat_close_input(&mAvFormatContext);
}

AVCodecParameters *Decoder::getVideoCodecParams() const
{
   return mAvFormatContext->streams[mVideoStreamIndex]->codecpar;
}

AVCodecParameters *Decoder::getAudioCodecParams() const
{
   return mAvFormatContext->streams[mAudioStremIndex]->codecpar;
}

int Decoder::getVideoStreamIndex() const
{
   return mVideoStreamIndex;
}
int Decoder::getAudioStreamIndex() const
{
   return mAudioStremIndex;
}

bool Decoder::readAvFrame(AVPacket &avPacket)
{
   //printf("I've read a frame\n");
   return av_read_frame(mAvFormatContext, &avPacket) >= 0;
}

bool Decoder::send(AVPacket *packet)
{
   //printf("I ve sent a packet\n");
   const int retVal = avcodec_send_packet(mAvCodecContext, packet);
   if (retVal == AVERROR(EAGAIN) || retVal == AVERROR_EOF)
   {
      return false;
   }
   else
   {
      DecoderUtils::checkFFMPEG(retVal);
      return true;
   }
}

bool Decoder::receive(AVFrame *frame)
{
   //printf("I ve received a frame\n");
   const int retVal = avcodec_receive_frame(mAvCodecContext, frame);
   if (retVal == AVERROR(EAGAIN) || retVal == AVERROR_EOF)
   {
      return false;
   }
   else
   {
      DecoderUtils::checkFFMPEG(retVal);
      return true;
   }
}

AVRational Decoder::getFormatTimeBase() const
{
   return mAvFormatContext->streams[mVideoStreamIndex]->time_base;
}

unsigned Decoder::getWidth() const
{
   return mAvCodecContext->width;
}
unsigned Decoder::getHeight() const
{
   return mAvCodecContext->height;
}

AVPixelFormat Decoder::getPixelFormat() const
{
   return mAvCodecContext->pix_fmt;
}

AVRational Decoder::getTimeBase() const
{
   return mAvCodecContext->time_base;
}

AVFormatContext *Decoder::getAvFormatContext() const
{
   return mAvFormatContext;
}