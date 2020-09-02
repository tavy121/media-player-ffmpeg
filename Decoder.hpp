#pragma once

#include "IDecoder.hpp"

class Decoder : public IDecoder
{
public:

    Decoder(const std::string & fileName);
    ~Decoder();

    AVCodecParameters* getVideoCodecParams() const override;
    AVCodecParameters* getAudioCodecParams() const override;
	int getVideoStreamIndex() const override;
    int getAudioStreamIndex() const override;
    bool readAvFrame(AVPacket &avPacket) override;
    bool send(AVPacket* packet) override;
	bool receive(AVFrame* frame) override;

	AVRational getFormatTimeBase() const override;

	unsigned getWidth() const override;
	unsigned getHeight() const override;
	AVPixelFormat getPixelFormat() const override;
	AVRational getTimeBase() const override;

    AVFormatContext *getAvFormatContext() const override;

private:
    AVCodecContext* mAvCodecContext;
    AVFormatContext* mAvFormatContext;
    int mVideoStreamIndex;
    int mAudioStremIndex;
};
