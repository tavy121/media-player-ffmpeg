#include "UserInterface.hpp"
#include <stdexcept>

#include "SDL2/SDL_image.h"


#include <sys/types.h>
#include <unistd.h>

SDLCreator::SDLCreator()
{
	checkSDL(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER), "init");
}

SDLCreator::~SDLCreator()
{
	SDL_Quit();
}

UserInterface::UserInterface(const std::string fileName, const unsigned width, const unsigned height) : mWindowSdl{checkSDL(SDL_CreateWindow(
	"player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
	"window"),
	SDL_DestroyWindow},
	mRendererSdl{checkSDL(SDL_CreateRenderer(
	mWindowSdl.get(), -1,
	SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
	"renderer"),
	SDL_DestroyRenderer},
	mTextureSdl{checkSDL(SDL_CreateTexture(
	mRendererSdl.get(), SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
	width, height),
	"renderer"),
	SDL_DestroyTexture}
{
	this->fileName = fileName;
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(mRendererSdl.get(), width, height);

	SDL_SetRenderDrawColor(mRendererSdl.get(), 0, 0, 0, 255);
	SDL_RenderClear(mRendererSdl.get());
	SDL_RenderPresent(mRendererSdl.get());

	SDL_GetWindowSize(mWindowSdl.get(), &windowWidth, &windowHeight);

	arrowCursorSurface = IMG_Load("Buttons/ArrowCursor.png");
	arrowCursor = SDL_CreateColorCursor(arrowCursorSurface, 0, 0);

	handCursorSurface = IMG_Load("Buttons/HandCursor.png");
	handCursor = SDL_CreateColorCursor(handCursorSurface, 12, 0);

	SDL_SetCursor(arrowCursor);

	playButtonTexture = SDL_CreateTextureFromSurface(mRendererSdl.get(), IMG_Load("Buttons/PlayButton1.png"));
	pauseButtonTexture = SDL_CreateTextureFromSurface(mRendererSdl.get(), IMG_Load("Buttons/PauseButton.png"));
	serverButtonTexture = SDL_CreateTextureFromSurface(mRendererSdl.get(), IMG_Load("Buttons/ServerButton.png"));
	clientButtonTexture = SDL_CreateTextureFromSurface(mRendererSdl.get(), IMG_Load("Buttons/ClientButton.png"));

	playButtonRect.x = (int)(windowWidth * 2 / 100);
	playButtonRect.y = (int)(windowHeight * 90 / 100);
	playButtonRect.w = (int)(windowHeight * 10 / 100);
	playButtonRect.h = (int)(windowHeight * 10 / 100);

	SDL_QueryTexture(playButtonTexture, nullptr, nullptr, &playPauseButtonX, &playPauseButtonY);
	
	serverButtonRect.x = (int)(windowWidth * 84 / 100);
	serverButtonRect.y = (int)(windowHeight * 90 / 100);
	serverButtonRect.w = (int)(windowHeight * 10 / 100);
	serverButtonRect.h = (int)(windowHeight * 10 / 100);

	SDL_QueryTexture(serverButtonTexture, nullptr, nullptr, &serverButtonX, &serverButtonY);

	clientButtonRect.x = (int)(windowWidth * 93 / 100);
	clientButtonRect.y = (int)(windowHeight * 90 / 100);
	clientButtonRect.w = (int)(windowHeight * 10 / 100);
	clientButtonRect.h = (int)(windowHeight * 10 / 100);

	SDL_QueryTexture(clientButtonTexture, nullptr, nullptr, &clientButtonX, &clientButtonY);
	
}

void UserInterface::refreshVideo(std::array<uint8_t *, 3> RGB, std::array<size_t, 3> videoLinesize)
{
	//printf("Update the video frame\n");
	checkSDL(!SDL_UpdateYUVTexture(
					 mTextureSdl.get(),
					 nullptr,
					 RGB[0], videoLinesize[0],
					 RGB[1], videoLinesize[1],
					 RGB[2], videoLinesize[2]),
				"Update frame");

	SDL_RenderClear(mRendererSdl.get());
	SDL_RenderCopy(mRendererSdl.get(), mTextureSdl.get(), nullptr, nullptr);
	this->drawPlayButton();
	this->drawServerButton();
	this->drawClientButton();
	SDL_RenderPresent(mRendererSdl.get());
}

void UserInterface::refreshSound(const uint8_t *soundPlane, const size_t &audioLinesize)
{
}

void UserInterface::input()
{

	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (SDL_PollEvent(&mEventSdl))
	{
		switch (mEventSdl.type)
		{
		case SDL_KEYUP:
			switch (mEventSdl.key.keysym.sym)
			{
			case SDLK_ESCAPE:
				{
					mQuit = true;
					break;
				}
			case SDLK_SPACE:
				{
					mPlay = !mPlay;
					break;
				}
			default:
				{
					break;
				}
			} // switch
		case SDL_MOUSEBUTTONDOWN:
		{
			if ((mouseX >= (int)(windowWidth * 2 / 100) && mouseX <= (int)(windowWidth * 2 / 100) + playButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + playButtonRect.h))
			{
				printf("I am at position X=%d Y=%d and playButtonX=%d, playPauseButtonY=%d and mPlay=%d\n",mouseX,mouseY, playPauseButtonX, playPauseButtonY, mPlay );
				mPlay = !mPlay;
			}
			else if ((mouseX >= (int)(windowWidth * 84 / 100) && mouseX <= (int)(windowWidth * 84 / 100) + serverButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + serverButtonRect.h))
			{
				printf("Server : I am at position X=%d Y=%d and playButtonX=%d, playPauseButtonY=%d and mPlay=%d\n",mouseX,mouseY, serverButtonX, serverButtonY, mPlay );
			}
			else if ((mouseX >= (int)(windowWidth * 93 / 100) && mouseX <= (int)(windowWidth * 93 / 100) + clientButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + clientButtonRect.h))
			{
				printf("Client : I am at position X=%d Y=%d and playButtonX=%d, playPauseButtonY=%d and mPlay=%d\n",mouseX,mouseY, clientButtonX, clientButtonY, mPlay );
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			if ((mouseX >= (int)(windowWidth * 2 / 100) && mouseX <= (int)(windowWidth * 2 / 100) + playButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + playButtonRect.h))
			{
				SDL_SetCursor(handCursor);
			}
			else if ((mouseX >= (int)(windowWidth * 84 / 100) && mouseX <= (int)(windowWidth * 84 / 100) + serverButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + serverButtonRect.h))
			{
				SDL_SetCursor(handCursor);
			}
			else if ((mouseX >= (int)(windowWidth * 93 / 100) && mouseX <= (int)(windowWidth * 93 / 100) + clientButtonRect.w) &&
				 (mouseY >= (int)(windowHeight * 90 / 100) && mouseY <= (int)(windowHeight * 90 / 100) + clientButtonRect.h))
			{
				SDL_SetCursor(handCursor);
			}
			else
			{
				SDL_SetCursor(arrowCursor);
			}
			break;
		}
		case SDL_QUIT:
			{
				mQuit = true;
				exit(0);
				break;
			}
		default:
			{
				break;
			}
		} // switch
	}
}

void UserInterface::handlePlayPauseButton()
{
}

void UserInterface::drawPlayButton()
{
	if (true == mPlay)
	{
		SDL_RenderCopy(mRendererSdl.get(), pauseButtonTexture, nullptr, &playButtonRect);
	}
	else
	{
		SDL_RenderCopy(mRendererSdl.get(), playButtonTexture, nullptr, &playButtonRect);
	}
}

void UserInterface::drawPauseButton()
{
}

void UserInterface::drawTimelineBar()
{
}

void UserInterface::drawOnAirSign()
{
}

void UserInterface::drawServerButton()
{
	SDL_RenderCopy(mRendererSdl.get(), serverButtonTexture, nullptr, &serverButtonRect);
}

void UserInterface::drawClientButton()
{
	SDL_RenderCopy(mRendererSdl.get(), clientButtonTexture, nullptr, &clientButtonRect);
}

bool UserInterface::getQuit() const
{
	return mQuit;
}

bool UserInterface::getPlay() const
{
	return mPlay;
}
