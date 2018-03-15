#include "conversion.h"
#include "image.h"
#include "../essentials/assert.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


void NImage::RGBToLumaChroma(SImage& image)
{
	MF_ASSERT(image.format == EFormat::RGB32);

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			SVector3* data = (SVector3*)image.data;
			int idx = Idx(x, y, image.width);

			data[idx] = RGBToLumaChroma(data[idx]);
		}
	}
}


void NImage::LumaChromaToRGB(SImage& image)
{
	MF_ASSERT(image.format == EFormat::RGB32);

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			SVector3* data = (SVector3*)image.data;
			int idx = Idx(x, y, image.width);

			data[idx] = LumaChromaToRGB(data[idx]);
		}
	}
}


SVector3 NImage::PixelToVector3_Inclusive(const SImage& image, int pixelIndex)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	SVector3 temp;

	temp.x = IntToNormalizedFloat_Inclusive(image.data[3*pixelIndex + 0]);
	temp.y = IntToNormalizedFloat_Inclusive(image.data[3*pixelIndex + 1]);
	temp.z = IntToNormalizedFloat_Inclusive(image.data[3*pixelIndex + 2]);

	return temp;
}


void NImage::Vector3ToPixel_Inclusive(const SVector3& color, SImage& image, int pixelIndex)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	image.data[3*pixelIndex + 0] = NormalizedFloatToInt_Inclusive(color.x);
	image.data[3*pixelIndex + 1] = NormalizedFloatToInt_Inclusive(color.y);
	image.data[3*pixelIndex + 2] = NormalizedFloatToInt_Inclusive(color.z);
}


SVector3 NImage::PixelToVector3(const SImage& image, int pixelIndex)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	SVector3 temp;

	temp.x = IntToNormalizedFloat(image.data[3*pixelIndex + 0]);
	temp.y = IntToNormalizedFloat(image.data[3*pixelIndex + 1]);
	temp.z = IntToNormalizedFloat(image.data[3*pixelIndex + 2]);

	return temp;
}


void NImage::Vector3ToPixel(const SVector3& color, SImage& image, int pixelIndex)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	image.data[3*pixelIndex + 0] = NormalizedFloatToInt(color.x);
	image.data[3*pixelIndex + 1] = NormalizedFloatToInt(color.y);
	image.data[3*pixelIndex + 2] = NormalizedFloatToInt(color.z);
}


SVector3 NImage::SVector3ui24ToVector3_Inclusive(const SVector3ui24& color)
{
	SVector3 temp;

	temp.x = IntToNormalizedFloat_Inclusive(color.x);
	temp.y = IntToNormalizedFloat_Inclusive(color.y);
	temp.z = IntToNormalizedFloat_Inclusive(color.z);

	return temp;
}


SVector3ui24 NImage::Vector3ToSVector3ui24_Inclusive(const SVector3& color)
{
	SVector3ui24 temp;

	temp.x = NormalizedFloatToInt_Inclusive(color.x);
	temp.y = NormalizedFloatToInt_Inclusive(color.y);
	temp.z = NormalizedFloatToInt_Inclusive(color.z);

	return temp;
}


NImage::SImage NImage::Convert(const SImage& image, EFormat conversionFormat)
{
	MF_ASSERT(
		(image.format == EFormat::R8 && conversionFormat == EFormat::RGBA8) ||
		(image.format == EFormat::RGB8 && conversionFormat == EFormat::RGBA8) ||
		(image.format == EFormat::RGBA8 && conversionFormat == EFormat::RGB8) ||
		(image.format == EFormat::R8 && conversionFormat == EFormat::R32) ||
		(image.format == EFormat::R32 && conversionFormat == EFormat::R8) ||
		(image.format == EFormat::RGB8 && conversionFormat == EFormat::RGB32) ||
		(image.format == EFormat::RGB32 && conversionFormat == EFormat::RGB8) );

	SImage convertedImage;

	if (image.format == EFormat::R8 && conversionFormat == EFormat::RGBA8)
	{
		convertedImage = Create(image.width, image.height, EFormat::RGBA8);

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);

				convertedImage.data[4*pixelIndex + 0] = image.data[pixelIndex];
				convertedImage.data[4*pixelIndex + 1] = image.data[pixelIndex];
				convertedImage.data[4*pixelIndex + 2] = image.data[pixelIndex];
				convertedImage.data[4*pixelIndex + 3] = image.data[pixelIndex];
			}
		}
	}
	else if (image.format == EFormat::RGB8 && conversionFormat == EFormat::RGBA8)
	{
		convertedImage = Create(image.width, image.height, EFormat::RGBA8);

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);

				convertedImage.data[4*pixelIndex + 0] = image.data[3*pixelIndex + 0];
				convertedImage.data[4*pixelIndex + 1] = image.data[3*pixelIndex + 1];
				convertedImage.data[4*pixelIndex + 2] = image.data[3*pixelIndex + 2];
				convertedImage.data[4*pixelIndex + 3] = 255;
			}
		}
	}
	else if (image.format == EFormat::RGBA8 && conversionFormat == EFormat::RGB8)
	{
		convertedImage = Create(image.width, image.height, EFormat::RGB8);

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);

				convertedImage.data[3*pixelIndex + 0] = image.data[4*pixelIndex + 0];
				convertedImage.data[3*pixelIndex + 1] = image.data[4*pixelIndex + 1];
				convertedImage.data[3*pixelIndex + 2] = image.data[4*pixelIndex + 2];
			}
		}
	}
	else if (image.format == EFormat::R8 && conversionFormat == EFormat::R32)
	{
		convertedImage = Create(image.width, image.height, EFormat::R32);
		float* convertedImageData = (float*)convertedImage.data;

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);
				convertedImageData[pixelIndex] = IntToNormalizedFloat_Inclusive(image.data[pixelIndex]);
			}
		}
	}
	else if (image.format == EFormat::R32 && conversionFormat == EFormat::R8)
	{
		convertedImage = Create(image.width, image.height, EFormat::R8);
		float* imageData = (float*)image.data;

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);
				convertedImage.data[pixelIndex] = NormalizedFloatToInt_Inclusive(imageData[pixelIndex]);
			}
		}
	}
	else if (image.format == EFormat::RGB8 && conversionFormat == EFormat::RGB32)
	{
		convertedImage = Create(image.width, image.height, EFormat::RGB32);

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);
				SVector3 pixel = PixelToVector3_Inclusive(image, pixelIndex);
				memcpy(convertedImage.data + pixelIndex*PixelSize(convertedImage), &pixel, sizeof(pixel));
			}
		}
	}
	else if (image.format == EFormat::RGB32 && conversionFormat == EFormat::RGB8)
	{
		convertedImage = Create(image.width, image.height, EFormat::RGB8);

		for (uint16 j = 0; j < image.height; j++)
		{
			for (uint16 i = 0; i < image.width; i++)
			{
				int pixelIndex = Idx(i, j, image.width);
				SVector3 pixel;
				memcpy(&pixel, image.data + pixelIndex*PixelSize(image), sizeof(pixel));
				Vector3ToPixel_Inclusive(pixel, convertedImage, pixelIndex);
			}
		}
	}

	return convertedImage;
}
