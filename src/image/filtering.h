#pragma once
#ifdef MAXEST_FRAMEWORK_DESKTOP


#include "types.h"
#include "../math/types.h"


namespace NMaxestFramework { namespace NImage
{
	NMath::SVector3 WeightedAverage(
		const SImage& image,
		int pixel0Index, float pixel0Weight,
		int pixel1Index, float pixel1Weight,
		int pixel2Index, float pixel2Weight,
		int pixel3Index, float pixel3Weight);
	NMath::SVector3 Gauss_Sigma2_Kernel7(const SImage& image, int pixelX, int pixelY);
	NMath::SVector3 BicubicHermite(const SImage& image, const NMath::SVector2& uv);
	vector<SImage> Simulate_BicubicHermiteDownscaleAndUpscale(const SImage& image, int scale);
} }


#endif
