#pragma once

#ifndef __DECODER_UTILS_V0__
#define __DECODER_UTILS_V0__

#include <array>
#include <stdexcept>
extern "C" 
{
	#include "libavutil/avutil.h"
}

class DecoderUtils
{
public:
    DecoderUtils(const std::string &message);
    DecoderUtils(const int status);
    static int checkFFMPEG(const int& returned);

};


#endif //__DECODER_UTILS_V0__
