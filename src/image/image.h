#pragma once


#include "types.h"
#include "../essentials/main.h"
#include "../math/types.h"


namespace NMaxestFramework { namespace NImage
{
	SImage Create(int width, int height, EFormat format);
	SImage Create(const SImage& image);
	SImage Copy(const SImage& image);

	void Destroy(SImage& image);

	bool Load(const string& path, SImage& image);
	bool Load(const string& path, SImage& image, bool swapChannels, bool invertY, EFormat format);
	bool Save(const string& path, const SImage& image, int flags = 0);
	bool Save(const string& path, const SImage& image, bool swapChannels, bool invertY, int flags = 0);
	bool SaveToMemory_JPEG50(const SImage& image, uint8*& data, int& dataSize);

	bool IsCompressed(EFormat format);
	bool IsCompressed(const SImage& image);
	bool IsFloat(EFormat format);
	bool IsFloat(const SImage& image);
	uint8 BPP(EFormat format);
	uint8 ChannelsCount(EFormat format);
	EFormat SingleChannelFormat(EFormat format);
	EFormat MultiChannelsFormat(EFormat singleChannelEFormat, int channelsCount);
	uint8 PixelSize(EFormat format);
	uint8 PixelSize(const SImage& image);
	uint32 Size(int width, int height, EFormat format);
	uint32 Size(const SImage& image);
	uint8 MipmapsCount(int width, int height);
	void SwapChannels(SImage& image, int firstChannelIndex, int secondChannelIndex);
	SImage Crop(const SImage& image, int cropX, int cropY, int cropWidth, int cropHeight);
	SImage Shift(const SImage& image, int shiftX, int shiftY);
	SImage InvertY(const SImage& image);
	SImage Scale(const SImage& image, int scaledWidth, int scaledHeight, EFilter filter);
	vector<SImage> GenerateMipmaps(const SImage& image, EFilter filter);
	void Saturate(SImage& image);
	void Difference(const SImage& image1, const SImage& image2, int offsetX, int offsetY, int width, int height, const NMath::SVector3& metric, float& total, float& maxPixel);
	void Difference(const SImage& image1, const SImage& image2, const NMath::SVector3& metric, float& total, float& maxPixel);

	vector<SImage> SplitChannels(const SImage& image);
	SImage MergeChannelImages(const vector<SImage>& images);

	SImage Compress(const SImage& image, EFormat compressionFormat);
	SImage Decompress(const SImage& image);

	SImage Checkerboard(const SImage& image, bool flip);
	SImage Decheckerboard(const SImage& image, bool flipped);
	void DecheckerboardFill(SImage& image, bool flipped);
	SImage DecheckerboardMerge(const SImage& image, const SImage& flippedImage);

	void AddIn(SImage& image, float value);
	void MulIn(SImage& image, float value);
	void PowIn(SImage& image, float value);
} }


#define SAFE_IMAGE_FUNCTION(image, function) \
	{ \
		SImage temp = image; \
		image = function; \
		delete[] temp.data; \
	}
