#include "main.h"

#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include "../../dependencies/SDL2-2.0.5/include/SDL.h"
#else
	#include "../../dependencies/SDL2-2.0/include/SDL.h"
#endif


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
