#include "fourier.h"
#include "image.h"
#include "../math/common.h"
#include "../math/types.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


float JPGAlpha(int u)
{
	if (u == 0)
		return 1.0f / Sqrt(2.0f);
	else
		return 1.0f;
}


NImage::SImage NImage::DiscreteFourierTransform(const SImage& image)
{
	int width = image.width;
	int height = image.height;
	SImage dftImage = Create(width, height, EFormat::RG32);
	SVector2* output = (SVector2*)dftImage.data;

	for (uint16 y = 0; y < height; y++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			output[Idx(x, y, width)].x = 0.0f;
			output[Idx(x, y, width)].y = 0.0f;

			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					float p = -cTwoPi * ((float)(x*i)/(float)width + (float)(y*j)/(float)height);

					float R = (float)image.data[Idx(i, j, width)];
					float I = 0.0f;

					output[Idx(x, y, width)].x += R*Cos(p) - I*Sin(p);
					output[Idx(x, y, width)].y += R*Sin(p) + I*Cos(p);
				}
			}
		}
	}

	return dftImage;
}


NImage::SImage NImage::DiscreteFourierTransform_Separable(const SImage& image)
{
	int width = image.width;
	int height = image.height;
	SImage dftImage = Create(width, height, EFormat::RG32);
	SVector2* output = (SVector2*)dftImage.data;
	SVector2* temp = new SVector2[width * height];

	for (uint16 y = 0; y < height; y++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			temp[Idx(x, y, width)].x = 0.0f;
			temp[Idx(x, y, width)].y = 0.0f;

			for (int i = 0; i < width; i++)
			{
				float p = -cTwoPi * ((float)(x*i)/(float)width);

				float R = (float)image.data[Idx(i, y, width)];
				float I = 0.0f;

				temp[Idx(x, y, width)].x += R*Cos(p) - I*Sin(p);
				temp[Idx(x, y, width)].y += R*Sin(p) + I*Cos(p);
			}
		}
	}

	for (uint16 y = 0; y < height; y++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			output[Idx(x, y, width)].x = 0.0f;
			output[Idx(x, y, width)].y = 0.0f;

			for (int j = 0; j < height; j++)
			{
				float p = -cTwoPi * ((float)(y*j)/(float)height);

				float& R = temp[Idx(x, j, width)].x;
				float& I = temp[Idx(x, j, width)].y;

				output[Idx(x, y, width)].x += R*Cos(p) - I*Sin(p);
				output[Idx(x, y, width)].y += R*Sin(p) + I*Cos(p);
			}
		}
	}

	delete[] temp;
	return dftImage;
}


NImage::SImage NImage::InverseDiscreteFourierTransform(const NImage::SImage& image)
{
	int width = image.width;
	int height = image.height;
	SImage idftImage = Create(width, height, EFormat::RG32);
	SVector2* output = (SVector2*)idftImage.data;
	SVector2* input = (SVector2*)image.data;

	for (uint16 y = 0; y < height; y++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			output[Idx(x, y, width)].x = 0.0f;
			output[Idx(x, y, width)].y = 0.0f;

			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					float p = cTwoPi * ((float)(x*i)/(float)width + (float)(y*j)/(float)height);

					float& R = input[Idx(i, j, width)].x;
					float& I = input[Idx(i, j, width)].y;

					output[Idx(x, y, width)].x += R*Cos(p) - I*Sin(p);
					output[Idx(x, y, width)].y += R*Sin(p) + I*Cos(p);
				}
			}

			output[Idx(x, y, width)].x /= (float)(width * height);
			output[Idx(x, y, width)].y /= (float)(width * height);
		}
	}

	return idftImage;
}


NImage::SImage NImage::InverseDiscreteFourierTransform_Separable(const SImage& image)
{
	int width = image.width;
	int height = image.height;
	SImage idftImage = Create(width, height, EFormat::RG32);
	SVector2* output = (SVector2*)idftImage.data;
	SVector2* input = (SVector2*)image.data;
	SVector2* temp = new SVector2[width * height];

	for (uint16 j = 0; j < height; j++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			temp[Idx(x, j, width)].x = 0.0f;
			temp[Idx(x, j, width)].y = 0.0f;

			for (int i = 0; i < width; i++)
			{
				float p = cTwoPi * ((float)(x*i)/(float)width);

				float& R = input[Idx(i, j, width)].x;
				float& I = input[Idx(i, j, width)].y;

				temp[Idx(x, j, width)].x += R*Cos(p) - I*Sin(p);
				temp[Idx(x, j, width)].y += R*Sin(p) + I*Cos(p);
			}

			temp[Idx(x, j, width)].x /= (float)width;
			temp[Idx(x, j, width)].y /= (float)width;
		}
	}

	for (uint16 y = 0; y < height; y++)
	{
		for (uint16 x = 0; x < width; x++)
		{
			output[Idx(x, y, width)].x = 0.0f;
			output[Idx(x, y, width)].y = 0.0f;

			for (int j = 0; j < height; j++)
			{
				float p = cTwoPi * ((float)(y*j)/(float)height);

				float& R = temp[Idx(x, j, width)].x;
				float& I = temp[Idx(x, j, width)].y;

				output[Idx(x, y, width)].x += R*Cos(p) - I*Sin(p);
				output[Idx(x, y, width)].y += R*Sin(p) + I*Cos(p);
			}

			output[Idx(x, y, width)].x /= (float)height;
			output[Idx(x, y, width)].y /= (float)height;
		}
	}

	delete[] temp;
	return idftImage;
}


float NImage::DCT_JPG(const SImage& image, int pixelX, int pixelY, int minX, int countX, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int u = pixelX - minX;
	int v = pixelY - minY;

	for (int y = 0; y < countY; y++)
	{
		for (int x = 0; x < countX; x++)
		{
			value +=
				imageData[Idx(minX + x, minY + y, image.width)] *
				Cos((2.0f*x + 1.0f)*u*cPi/16.0f) * 
				Cos((2.0f*y + 1.0f)*v*cPi/16.0f);
		}
	}

	return 0.25f * JPGAlpha(u) * JPGAlpha(v) * value;
}


float NImage::IDCT_JPG(const SImage& image, int pixelX, int pixelY, int minX, int countX, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int u = pixelX - minX;
	int v = pixelY - minY;

	for (int y = 0; y < countY; y++)
	{
		for (int x = 0; x < countX; x++)
		{
			value +=
				JPGAlpha(x) * JPGAlpha(y) *
				imageData[Idx(minX + x, minY + y, image.width)] *
				Cos((2.0f*u + 1.0f)*x*cPi/16.0f) * 
				Cos((2.0f*v + 1.0f)*y*cPi/16.0f);
		}
	}

	return 0.25f * value;
}


float NImage::DCT_JPEG_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int u = pixelX - minX;

	for (int x = 0; x < countX; x++)
		value += imageData[Idx(minX + x, pixelY, image.width)] * Cos((2.0f*x + 1.0f)*u*cPi/16.0f);

	return 0.5f * JPGAlpha(u) * value;
}


float NImage::DCT_JPEG_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int v = pixelY - minY;

	for (int y = 0; y < countY; y++)
		value += imageData[Idx(pixelX, minY + y, image.width)] * Cos((2.0f*y + 1.0f)*v*cPi/16.0f);

	return 0.5f * JPGAlpha(v) * value;
}


float NImage::IDCT_JPEG_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int u = pixelX - minX;

	for (int x = 0; x < countX; x++)
		value += JPGAlpha(x) * imageData[Idx(minX + x, pixelY, image.width)] * Cos((2.0f*u + 1.0f)*x*cPi/16.0f);

	return 0.5f * value;
}


float NImage::IDCT_JPEG_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	int v = pixelY - minY;

	for (int y = 0; y < countY; y++)
		value += JPGAlpha(y) * imageData[Idx(pixelX, minY + y, image.width)] * Cos((2.0f*v + 1.0f)*y*cPi/16.0f);

	return 0.5f * value;
}


float NImage::DCT_II_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	float value = 0.0f;
	float* imageData = (float*)image.data;

	for (int x = 0; x < countX; x++)
		value += imageData[Idx(minX + x, pixelY, image.width)] * Cos(cPi/countX * (x + 0.5f) * (pixelX - minX));

	return value;
}


float NImage::DCT_II_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	for (int y = 0; y < countY; y++)
		value += imageData[Idx(pixelX, minY + y, image.width)] * Cos(cPi/countY * (y + 0.5f) * (pixelY - minY));

	return value;
}


float NImage::IDCT_II_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	float* imageData = (float*)image.data;
	float value = 0.5f * imageData[Idx(minX, pixelY, image.width)];

	for (int x = 1; x < countX; x++)
		value += imageData[Idx(minX + x, pixelY, image.width)] * Cos(cPi/countX * x * (0.5f + (pixelX - minX)));

	return 2.0f / countX * value;
}


float NImage::IDCT_II_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.5f * imageData[Idx(pixelX, minY, image.width)];

	for (int y = 1; y < countY; y++)
		value += imageData[Idx(pixelX, minY + y, image.width)] * Cos(cPi/countY * y * (0.5f + (pixelY - minY)));

	return 2.0f / countY * value;
}


float NImage::DCT_IV_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	for (int x = 0; x < countX; x++)
		value += imageData[Idx(minX + x, pixelY, image.width)] * Cos(cPi/countX * (x + 0.5f) * ((pixelX - minX) + 0.5f));

	return value;
}


float NImage::DCT_IV_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	float* imageData = (float*)image.data;
	float value = 0.0f;

	for (int y = 0; y < countY; y++)
		value += imageData[Idx(pixelX, minY + y, image.width)] * Cos(cPi/countY * (y + 0.5f) * ((pixelY - minY) + 0.5f));

	return value;
}


float NImage::IDCT_IV_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX)
{
	return 2.0f / countX * DCT_IV_Horizontal(image, pixelX, pixelY, minX, countX);
}


float NImage::IDCT_IV_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY)
{
	return 2.0f / countY * DCT_IV_Vertical(image, pixelX, pixelY, minY, countY);
}


void NImage::Block(SImage& dstImage, const SImage& srcImage, int blockWidth, int blockHeight, int blockX, int blockY, TBlockFunction function)
{
	MF_ASSERT(dstImage.format == EFormat::R32);
	MF_ASSERT(srcImage.format == EFormat::R32);
	MF_ASSERT(dstImage.width == srcImage.width && dstImage.height == srcImage.height);
	MF_ASSERT(dstImage.width % blockWidth == 0 && dstImage.height % blockHeight == 0);

	int minX = blockX * blockWidth;
	int minY = blockY * blockHeight;

	float* dstImageData = (float*)dstImage.data;

	for (int y = 0; y < blockHeight; y++)
	{
		for (int x = 0; x < blockWidth; x++)
		{
			int idx = Idx(minX + x, minY + y, dstImage.width);
			dstImageData[idx] = function(srcImage, minX + x, minY + y, minX, blockWidth, minY, blockHeight);
		}
	}
}


NImage::SImage NImage::Blocks(const SImage& image, int blockWidth, int blockHeight, TBlockFunction function)
{
	MF_ASSERT(image.format == EFormat::R32);
	MF_ASSERT(image.width % blockWidth == 0 && image.height % blockHeight == 0);

	int blocksX = image.width / blockWidth;
	int blocksY = image.height / blockHeight;

	SImage resultImage = Create(image);

	for (int y = 0; y < blocksY; y++)
	{
		for (int x = 0; x < blocksX; x++)
		{
			Block(resultImage, image, blockWidth, blockHeight, x, y, function);
		}
	}

	return resultImage;
}


void NImage::Block_Separable(SImage& dstImage, const SImage& srcImage, int blockWidth, int blockHeight, int blockX, int blockY, TBlockFunction_Separable function, int min, int count)
{
	MF_ASSERT(dstImage.format == EFormat::R32);
	MF_ASSERT(srcImage.format == EFormat::R32);
	MF_ASSERT(dstImage.width == srcImage.width && dstImage.height == srcImage.height);
	MF_ASSERT(dstImage.width % blockWidth == 0 && dstImage.height % blockHeight == 0);

	int minX = blockX * blockWidth;
	int minY = blockY * blockHeight;

	float* dstImageData = (float*)dstImage.data;

	for (int y = 0; y < blockHeight; y++)
	{
		for (int x = 0; x < blockWidth; x++)
		{
			int idx = Idx(minX + x, minY + y, dstImage.width);
			dstImageData[idx] = function(srcImage, minX + x, minY + y, min, count);
		}
	}
}


NImage::SImage NImage::Blocks_Horizontal(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable function)
{
	MF_ASSERT(image.format == EFormat::R32);
	MF_ASSERT(image.width % blockWidth == 0 && image.height % blockHeight == 0);

	int blocksX = image.width / blockWidth;
	int blocksY = image.height / blockHeight;

	SImage resultImage = Create(image);

	for (int y = 0; y < blocksY; y++)
	{
		for (int x = 0; x < blocksX; x++)
		{
			Block_Separable(resultImage, image, blockWidth, blockHeight, x, y, function, x*blockWidth, blockWidth);
		}
	}

	return resultImage;
}


NImage::SImage NImage::Blocks_Vertical(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable function)
{
	MF_ASSERT(image.format == EFormat::R32);
	MF_ASSERT(image.width % blockWidth == 0 && image.height % blockHeight == 0);

	int blocksX = image.width / blockWidth;
	int blocksY = image.height / blockHeight;

	SImage resultImage = Create(image);

	for (int y = 0; y < blocksY; y++)
	{
		for (int x = 0; x < blocksX; x++)
		{
			Block_Separable(resultImage, image, blockWidth, blockHeight, x, y, function, y*blockHeight, blockHeight);
		}
	}

	return resultImage;
}


NImage::SImage NImage::Blocks(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable horizontalFunction, TBlockFunction_Separable verticalFunction)
{
	MF_ASSERT(image.format == EFormat::R32);
	MF_ASSERT(image.width % blockWidth == 0 && image.height % blockHeight == 0);

	SImage resultImage = Create(image);
	SImage tempImage = Create(image);

	tempImage = Blocks_Horizontal(image, blockWidth, blockHeight, horizontalFunction);
	resultImage = Blocks_Vertical(tempImage, blockWidth, blockHeight, verticalFunction);

	delete[] tempImage.data;
	return resultImage;
}
