#pragma once


namespace NMaxestFramework
{
	namespace NSystem {}
}


#include "threads.h"
#include "application.h"
#include "keys.h"
#include "sockets.h"
#include "common.h"


namespace NMaxestFramework { namespace NSystem
{
	void Initialize();
	void Deinitialize();
} }
