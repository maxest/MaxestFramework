#pragma once


#include "types.h"
#include "../essentials/main.h"


namespace NMaxestFramework { namespace NImage
{
	// Fourier

	SImage DiscreteFourierTransform(const SImage& image);
	SImage DiscreteFourierTransform_Separable(const SImage& image);
	SImage InverseDiscreteFourierTransform(const SImage& image);
	SImage InverseDiscreteFourierTransform_Separable(const SImage& image);

	// DCT
	// https://en.wikipedia.org/wiki/Discrete_cosine_transform

	typedef float (*TBlockFunction_Separable)(const SImage& image, int pixelX, int pixelY, int min, int count);
	typedef float (*TBlockFunction)(const SImage& image, int pixelX, int pixelY, int minX, int countX, int minY, int countY);

	float DCT_JPG(const SImage& image, int pixelX, int pixelY, int minX, int countX, int minY, int countY);
	float IDCT_JPG(const SImage& image, int pixelX, int pixelY, int minX, int countX, int minY, int countY);
	float DCT_JPEG_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX);
	float DCT_JPEG_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY);
	float IDCT_JPEG_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX);
	float IDCT_JPEG_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY);

	float DCT_II_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX);
	float DCT_II_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int maxY);
	float IDCT_II_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int maxX);
	float IDCT_II_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int maxY);
	float DCT_IV_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX);
	float DCT_IV_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY);
	float IDCT_IV_Horizontal(const SImage& image, int pixelX, int pixelY, int minX, int countX);
	float IDCT_IV_Vertical(const SImage& image, int pixelX, int pixelY, int minY, int countY);

	void Block(SImage& dstImage, const SImage& srcImage, int blockWidth, int blockHeight, int blockX, int blockY, TBlockFunction function);
	SImage Blocks(const SImage& image, int blockWidth, int blockHeight, TBlockFunction function);
	void Block_Separable(SImage& dstImage, const SImage& srcImage, int blockWidth, int blockHeight, int blockX, int blockY, TBlockFunction_Separable function, int min, int count);
	SImage Blocks_Horizontal(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable function);
	SImage Blocks_Vertical(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable function);
	SImage Blocks(const SImage& image, int blockWidth, int blockHeight, TBlockFunction_Separable horizontalFunction, TBlockFunction_Separable verticalFunction);
} }
