#include "Utils.hpp"

#include <algorithm>
#include <thread>

Converter::Converter(size_t width, size_t height, AVPixelFormat input_pixel_format, AVPixelFormat output_pixel_format)
{
    this->width = width;
    this->height = height;
    mConversionContext = sws_getContext(width, height, input_pixel_format, width, height, output_pixel_format, SWS_BICUBIC, nullptr, nullptr, nullptr);
}

void Converter::operator()(AVFrame* src, AVFrame* dst)
{
    int ok = sws_scale(mConversionContext, src->data, src->linesize, 0, height, dst->data, dst->linesize);
    if(ok < 0)
    {
        std::__throw_runtime_error("Error in sws_scale");
    }
}

Timer::Timer() :
	target_time_{std::chrono::high_resolution_clock::now()} {
}

void Timer::wait(const int64_t period) {
	target_time_ += std::chrono::microseconds{period};

	const auto lag =
		std::chrono::duration_cast<std::chrono::microseconds>(
			target_time_ - std::chrono::high_resolution_clock::now()) +
			std::chrono::microseconds{adjust()};

	std::this_thread::sleep_for(lag);

	const int64_t error =
		std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now() - target_time_).count();
	derivative_ = error - proportional_;
	integral_ += error;
	proportional_ = error;

}

void Timer::update() {
	target_time_ = std::chrono::high_resolution_clock::now();
}

int64_t Timer::adjust() const {
	return P_ * proportional_ + I_ * integral_ + D_ * derivative_;
}
