#include <memory>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <SDL3/SDL.h>

#include "Log.h"
#include "GameBoy.h"

int main(int argc, char* argv[])
{
    // Remap clog to file
    std::ofstream ofs("CPU.log");
    std::clog.rdbuf(ofs.rdbuf());

	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		std::string errorStr = "SDL Error while creating a window";
		errorStr += SDL_GetError();
		Log::LogError(errorStr.c_str());
    	std::getchar(); // Prevent console from closing

		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("BitDMG - ", 160*2, 144*2, SDL_WINDOW_RESIZABLE);
	if(window == nullptr)
	{
		std::string errorStr = "SDL Error while creating a window";
		errorStr += SDL_GetError();
		Log::LogError(errorStr.c_str());
    	std::getchar(); // Prevent console from closing

		SDL_Quit();
		return 1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if(renderer == nullptr)
	{
		std::string errorStr = "SDL Error while creating a renderer";
		errorStr += SDL_GetError();
		Log::LogError(errorStr.c_str());
    	std::getchar(); // Prevent console from closing

		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}
	
	SDL_SetRenderVSync(renderer, 1);

    std::filesystem::path romPath;
    if (argc == 2) romPath = argv[1];
	else romPath = "Tetris.gb";

    GameBoy gb = {romPath, window};
    if (!gb.IsValid()) 
	{
		Log::LogCustom("Shuting down SDL", "SDL");
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		
		SDL_Quit();
		return 1;
	}

    while(gb.IsRunning())
    {
        gb.Update();
    }

	Log::LogCustom("Shuting down SDL", "SDL");
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
