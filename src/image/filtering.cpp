#include "image.h"
#include "filtering.h"
#include "conversion.h"
#include "../essentials/macros.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


SVector3 NImage::WeightedAverage(
	const SImage& image,
	int pixel0Index, float pixel0Weight,
	int pixel1Index, float pixel1Weight,
	int pixel2Index, float pixel2Weight,
	int pixel3Index, float pixel3Weight)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	SVector3 pixel0 = PixelToVector3(image, pixel0Index);
	SVector3 pixel1 = PixelToVector3(image, pixel1Index);
	SVector3 pixel2 = PixelToVector3(image, pixel2Index);
	SVector3 pixel3 = PixelToVector3(image, pixel3Index);

	return pixel0Weight*pixel0 + pixel1Weight*pixel1 + pixel2Weight*pixel2 + pixel3Weight*pixel3;
}


// http://dev.theomader.com/gaussian-kernel-calculator/
SVector3 NImage::Gauss_Sigma2_Kernel7(const SImage& image, int pixelX, int pixelY)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	static float kernel[7][7] =
	{
		0.005084f, 0.009377f, 0.013539f, 0.015302f, 0.013539f, 0.009377f, 0.005084f,
		0.009377f, 0.017296f, 0.024972f, 0.028224f, 0.024972f, 0.017296f, 0.009377f,
		0.013539f, 0.024972f, 0.036054f, 0.040749f, 0.036054f, 0.024972f, 0.013539f,
		0.015302f, 0.028224f, 0.040749f, 0.046056f, 0.040749f, 0.028224f, 0.015302f,
		0.013539f, 0.024972f, 0.036054f, 0.040749f, 0.036054f, 0.024972f, 0.013539f,
		0.009377f, 0.017296f, 0.024972f, 0.028224f, 0.024972f, 0.017296f, 0.009377f,
		0.005084f, 0.009377f, 0.013539f, 0.015302f, 0.013539f, 0.009377f, 0.005084f,
	};

	SVector3 result = cVector3Zero;

	for (int y = 0; y < 7; y++)
	{
		for (int x = 0; x < 7; x++)
		{
			int sampleX = Clamp(pixelX + x - 3, 0, image.width - 1);
			int sampleY = Clamp(pixelY + y - 3, 0, image.height - 1);

			result += kernel[x][y] * PixelToVector3(image, Idx(sampleX, sampleY, image.width));
		}
	}

	return result;
}


// https://www.shadertoy.com/view/MllSzX
SVector3 NImage::BicubicHermite(const SImage& image, const SVector2& uv)
{
	#define CX(x) (Clamp(x, 0, image.width - 1))
	#define CY(x) (Clamp(x, 0, image.height - 1))

	struct Utils
	{
		SVector3 CubicHermite(const SVector3& p1, const SVector3& p2, const SVector3& p3, const SVector3& p4, float t)
		{
			float t2 = t*t;
			float t3 = t*t*t;

			SVector3 a = -p1/2.0f + (3.0f*p2)/2.0f - (3.0f*p3)/2.0f + p4/2.0f;
			SVector3 b = p1 - (5.0f*p2)/2.0f + 2.0f*p3 - p4/2.0f;
			SVector3 c = -p1/2.0f + p3/2.0f;
   			SVector3 d = p2;

			return a*t3 + b*t2 + c*t + d;
		}
	} utils;

	MF_ASSERT(image.format == EFormat::RGB32);

    SVector2 uv_image = uv*VectorCustom((float)image.width, (float)image.height) - VectorCustom(0.5f, 0.5f);
	int x = (int)uv_image.x;
	int y = (int)uv_image.y;
	float fracX = Frac(uv_image.x);
	float fracY = Frac(uv_image.y);

	SVector3* data = (SVector3*)image.data;
	SVector3 c00 = data[Idx(CX(x-1), CY(y-1), image.width)];
	SVector3 c10 = data[Idx(CX(x-0), CY(y-1), image.width)];
	SVector3 c20 = data[Idx(CX(x+1), CY(y-1), image.width)];
	SVector3 c30 = data[Idx(CX(x+2), CY(y-1), image.width)];
	SVector3 c01 = data[Idx(CX(x-1), CY(y-0), image.width)];
	SVector3 c11 = data[Idx(CX(x-0), CY(y-0), image.width)];
	SVector3 c21 = data[Idx(CX(x+1), CY(y-0), image.width)];
	SVector3 c31 = data[Idx(CX(x+2), CY(y-0), image.width)];
	SVector3 c02 = data[Idx(CX(x-1), CY(y+1), image.width)];
	SVector3 c12 = data[Idx(CX(x-0), CY(y+1), image.width)];
	SVector3 c22 = data[Idx(CX(x+1), CY(y+1), image.width)];
	SVector3 c32 = data[Idx(CX(x+2), CY(y+1), image.width)];
	SVector3 c03 = data[Idx(CX(x-1), CY(y+2), image.width)];
	SVector3 c13 = data[Idx(CX(x-0), CY(y+2), image.width)];
	SVector3 c23 = data[Idx(CX(x+1), CY(y+2), image.width)];
	SVector3 c33 = data[Idx(CX(x+2), CY(y+2), image.width)];

    return utils.CubicHermite(
		utils.CubicHermite(c00, c10, c20, c30, fracX),
		utils.CubicHermite(c01, c11, c21, c31, fracX),
		utils.CubicHermite(c02, c12, c22, c32, fracX),
		utils.CubicHermite(c03, c13, c23, c33, fracX),
		fracY);

	#undef CX
	#undef CY
}


vector<NImage::SImage> NImage::Simulate_BicubicHermiteDownscaleAndUpscale(const SImage& image, int scale)
{
	SImage tempImage1 = NImage::Scale(image, image.width/scale, image.height/scale, NImage::EFilter::BiCubic);
	SImage tempImage2 = Convert(tempImage1, EFormat::RGB32);
	RGBToLumaChroma(tempImage2);
	Saturate(tempImage2);

	SImage tempImage3 = Create(image.width, image.height, EFormat::RGB32);
	SVector3* data = (SVector3*)tempImage3.data;
	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			SVector2 uv = VectorCustom((float)x/image.width, (float)y/image.height);
			data[Idx(x, y, tempImage3.width)] = Clamp(BicubicHermite(tempImage2, uv), cVector3Zero, cVector3One);
		}
	}

	vector<SImage> images = SplitChannels(tempImage3);

	delete[] tempImage1.data;
	delete[] tempImage2.data;
	delete[] tempImage3.data;

	return images;
}
