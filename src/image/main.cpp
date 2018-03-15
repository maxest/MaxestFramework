#include "main.h"
#include "../essentials/main.h"
#include "../system/common.h"

#include "../../dependencies/FreeImage-3.17.0/include/FreeImage.h"


using namespace NMaxestFramework;


void NImage::Initialize()
{
	FreeImage_Initialise();
}


void NImage::Deinitialize()
{
	FreeImage_DeInitialise();
}
