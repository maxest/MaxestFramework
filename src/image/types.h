#pragma once
#ifdef MAXEST_FRAMEWORK_DESKTOP


#include "../essentials/main.h"
#include "../common/lossless_compression.h"


namespace NMaxestFramework { namespace NImage
{
	enum class EFormat { Unknown, R8, RG8, RGB8, RGBA8, R32, RG32, RGB32, RGBA32, DXT1, DXT5 };
	enum class EFilter { Box, BiCubic, Bilinear, BSpline, CatmullRom, Lanczos3 };

	struct SImage
	{
		uint8* data;
		uint16 width, height;
		EFormat format;

		SImage()
		{
			data = nullptr;
		}
	};
} }


#endif
