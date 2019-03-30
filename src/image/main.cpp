#include "main.h"
#include "../essentials/main.h"
#include "../system/common.h"

#ifdef MAXEST_FRAMEWORK_DESKTOP
#include "../../dependencies/FreeImage-3.17.0/include/FreeImage.h"
#endif


using namespace NMaxestFramework;


void NImage::Initialize()
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	FreeImage_Initialise();
#endif
}


void NImage::Deinitialize()
{
#ifdef MAXEST_FRAMEWORK_DESKTOP
	FreeImage_DeInitialise();
#endif
}
