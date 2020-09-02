#pragma once

//C++ includes
#include <array>
#include <memory>

//SDL2
#include "SDL2/SDL.h"

class IUserInterface
{
 public:

   virtual void refreshVideo(std::array<uint8_t *, 3> RGB, std::array<size_t, 3> videoLinesize) = 0;
   virtual void refreshSound(const uint8_t *soundPlane, const size_t &audioLinesize) = 0;

   virtual void input() = 0;
   virtual void handlePlayPauseButton() = 0;
   virtual void drawPlayButton() = 0;
   virtual void drawPauseButton() = 0;
   virtual void drawTimelineBar() = 0;
   virtual void drawOnAirSign() = 0;

   virtual void drawServerButton() = 0;
   virtual void drawClientButton() = 0;

   virtual bool getQuit() const = 0;
   virtual bool getPlay() const = 0;
};
