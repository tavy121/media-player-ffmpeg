#pragma once

//C++ includes
#include <string>

//Local includes
#include "DecoderUtils.hpp"

//FFMPEG
extern "C" {
	#include "libavformat/avformat.h"
	#include "libavcodec/avcodec.h"
}

class IDecoder
{
public:

    virtual AVCodecParameters* getVideoCodecParams() const = 0;
    virtual AVCodecParameters* getAudioCodecParams() const = 0;
	virtual int getVideoStreamIndex() const = 0;
    virtual int getAudioStreamIndex() const = 0;
    virtual bool readAvFrame(AVPacket &avPacket) = 0;
    virtual bool send(AVPacket* packet) = 0;
	virtual bool receive(AVFrame* frame) = 0;

	virtual AVRational getFormatTimeBase() const = 0;

	virtual unsigned getWidth() const = 0;
	virtual unsigned getHeight() const = 0;
	virtual AVPixelFormat getPixelFormat() const = 0;
	virtual AVRational getTimeBase() const = 0;

    virtual AVFormatContext *getAvFormatContext() const = 0;
};