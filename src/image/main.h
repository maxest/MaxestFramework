#pragma once
#ifdef MAXEST_FRAMEWORK_DESKTOP


namespace NMaxestFramework
{
	namespace NImage {}
}


#include "types.h"
#include "image.h"
#include "filtering.h"
#include "conversion.h"
#include "fourier.h"


namespace NMaxestFramework { namespace NImage
{
	void Initialize();
	void Deinitialize();
} }


#endif
