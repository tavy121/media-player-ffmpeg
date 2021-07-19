#pragma once

#include <stdexcept>

#include "IUserInterface.hpp"


class SDLCreator
{
 public:
   SDLCreator();
   ~SDLCreator();
};

class UserInterface : public IUserInterface
{
 public:
   UserInterface(const std::string fileName, const unsigned width, const unsigned height);

   void refreshVideo(std::array<uint8_t *, 3> RGB, std::array<size_t, 3> videoLinesize) override;
   void refreshSound(const uint8_t *soundPlane, const size_t &audioLinesize) override;

   void input() override;
   void handlePlayPauseButton() override;
   void drawPlayButton() override;
   void drawPauseButton() override;
   void drawTimelineBar() override;
   void drawOnAirSign() override;

   void drawServerButton() override;
   void drawClientButton() override;

   bool getQuit() const override;
   bool getPlay() const override;

 private:
   bool mQuit{false};
   bool mPlay{true};

   int windowWidth = 0;
   int windowHeight = 0;

   int playPauseButtonX;
   int playPauseButtonY;

   int serverButtonX;
   int serverButtonY;

   int clientButtonX;
   int clientButtonY;

   int currentPortNumber = 8080;
   int currentPortNumberClient = 8080;

   std::string fileName;

   SDLCreator mSdl; //must be first
   std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> mWindowSdl;
   std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> mRendererSdl;
   std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> mTextureSdl;

   SDL_Event mEventSdl;

   SDL_Cursor * handCursor;
	SDL_Cursor * arrowCursor;

   SDL_Surface * arrowCursorSurface;
   SDL_Surface * handCursorSurface;

   SDL_Texture * playButtonTexture;
   SDL_Texture * pauseButtonTexture;

   SDL_Texture * serverButtonTexture;
   SDL_Texture * clientButtonTexture;

   SDL_Rect playButtonRect;
   SDL_Rect serverButtonRect;
   SDL_Rect clientButtonRect;
};

template <typename T>
inline T checkSDL(T value, const std::string &message)
{
   if (!value)
   {
      throw std::runtime_error{"SDL returned" + message};
   }
   else
   {
      return value;
   }
}
