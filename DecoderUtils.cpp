#include "DecoderUtils.hpp"

DecoderUtils::DecoderUtils(const std::string &message)
{
    std::runtime_error("FFMPEG error " + message);
}

DecoderUtils::DecoderUtils(const int status)
{
    std::runtime_error("FFMPEG error " + status);
}

int DecoderUtils::checkFFMPEG(const int& returned)
{
    if(returned < 0)
    {
        throw DecoderUtils(returned);
    }
    return returned;
}
