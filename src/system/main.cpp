#include "main.h"

#ifdef MAXEST_FRAMEWORK_DESKTOP
	#ifdef MAXEST_FRAMEWORK_WINDOWS
		#include "../../dependencies/SDL2-2.0.5/include/SDL.h"
	#else
		#include "../../dependencies/SDL2-2.0/include/SDL.h"
	#endif
#endif


using namespace NMaxestFramework;


void NSystem::Initialize()
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	int returnCode = SDL_Init(SDL_INIT_VIDEO);
	MF_ASSERT(returnCode >= 0);
#endif
}


void NSystem::Deinitialize()
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	SDL_Quit();
#endif
}
