#include "main.h"

#include "../../dependencies/SDL2-2.0.5/include/SDL.h"


using namespace NMaxestFramework;


void NSystem::Initialize()
{
	int returnCode = SDL_Init(SDL_INIT_VIDEO);
	MF_ASSERT(returnCode >= 0);
}


void NSystem::Deinitialize()
{
	SDL_Quit();
}
