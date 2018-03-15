#pragma once


#include "types.h"
#include "../math/types.h"


namespace NMaxestFramework { namespace NImage
{
	void RGBToLumaChroma(SImage& image);
	void LumaChromaToRGB(SImage& image);

	NMath::SVector3 PixelToVector3_Inclusive(const SImage& image, int pixelIndex);
	void Vector3ToPixel_Inclusive(const NMath::SVector3& color, SImage& image, int pixelIndex);
	NMath::SVector3 PixelToVector3(const SImage& image, int pixelIndex);
	void Vector3ToPixel(const NMath::SVector3& color, SImage& image, int pixelIndex);
	NMath::SVector3 SVector3ui24ToVector3_Inclusive(const NMath::SVector3ui24& color);
	NMath::SVector3ui24 Vector3ToSVector3ui24_Inclusive(const NMath::SVector3& color);

	SImage Convert(const SImage& image, EFormat conversionFormat);
} }
