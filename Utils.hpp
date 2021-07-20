#pragma once
#ifndef __UTILS_V0__
#define __UTILS_V0__

#include <cstdint>
#include <chrono>

extern "C" {
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"
}

class Converter
{
public:
    Converter(size_t width, size_t height, AVPixelFormat input_pixel_format, AVPixelFormat output_pixel_format);
    void operator()(AVFrame* src, AVFrame* dst);

private:
    SwsContext* mConversionContext;
    size_t width;
    size_t height;
};

class Timer {
public:
	Timer();
	void wait(int64_t period);
	void update();

private:
	int64_t adjust() const;

	std::chrono::time_point<std::chrono::high_resolution_clock> target_time_;

	int64_t proportional_{};
	int64_t integral_{};
	int64_t derivative_{};

	constexpr static double P_{0.0};
	constexpr static double I_{-1.0};
	constexpr static double D_{0.0};
};


#endif //__UTILS_V0__