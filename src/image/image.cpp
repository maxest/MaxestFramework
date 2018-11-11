#include "image.h"
#include "filtering.h"
#include "conversion.h"
#include "../essentials/macros.h"
#include "../math/main.h"

#include "../../dependencies/FreeImage-3.17.0/include/FreeImage.h"
#ifdef MAXEST_FRAMEWORK_WINDOWS
	#include "../../dependencies/squish-1.11/include/squish.h"
#else
	#include "../../dependencies/squish-1.15/include/squish.h"
#endif


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


NImage::SImage NImage::Create(int width, int height, EFormat format)
{
	SImage image;

	image.width = width;
	image.height = height;
	image.format = format;
	image.data = new uint8[Size(width, height, format)];

	return image;
}


NImage::SImage NImage::Create(const SImage& image)
{
	return Create(image.width, image.height, image.format);
}


NImage::SImage NImage::Copy(const SImage& image)
{
	SImage copy = Create(image);
	memcpy(copy.data, image.data, Size(image));
	return copy;
}


void NImage::Destroy(SImage& image)
{
	SAFE_DELETE_ARRAY(image.data);
}


bool NImage::Load(const string& path, SImage& image)
{
	FREE_IMAGE_FORMAT fif;
	FIBITMAP* fib;

	fif = FreeImage_GetFileType(path.c_str());
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilename(path.c_str());

	if (fif != FIF_BMP && fif != FIF_JPEG && fif != FIF_JP2 && fif != FIF_PNG && fif != FIF_TARGA)
		return false;

	if (!FreeImage_FIFSupportsReading(fif))
		return false;

	fib = FreeImage_Load(fif, path.c_str());

	if (!fib)
		return false;

	image.width = FreeImage_GetWidth(fib);
	image.height = FreeImage_GetHeight(fib);
	if (FreeImage_GetBPP(fib) == 8)
	{
		image.format = EFormat::R8;
	}
	else if (FreeImage_GetBPP(fib) == 16)
	{
		image.format = EFormat::RG8;
	}
	else if (FreeImage_GetBPP(fib) == 24)
	{
		image.format = EFormat::RGB8;
	}
	else if (FreeImage_GetBPP(fib) == 32)
	{
		image.format = EFormat::RGBA8;
	}
	else
	{
		MF_ASSERT(false);
	}

	uint32 dataSize = image.width * image.height * ChannelsCount(image.format);
	image.data = new uint8[dataSize];
	memcpy(image.data, FreeImage_GetBits(fib), dataSize);

	FreeImage_Unload(fib);

	return true;
}


bool NImage::Load(const string& path, SImage& image, bool swapChannels, bool invertY, EFormat format)
{
	if (!Load(path, image))
		return false;

	if (swapChannels)
	{
		if (image.format == EFormat::RGB8 || image.format == EFormat::RGBA8)
			SwapChannels(image, 0, 2);
	}

	if (invertY)
		SAFE_IMAGE_FUNCTION(image, InvertY(image));

	if (format != EFormat::Unknown && image.format != format)
		SAFE_IMAGE_FUNCTION(image, Convert(image, format));

	return true;
}


bool NImage::Save(const string& path, const SImage& image, int flags)
{
	FREE_IMAGE_FORMAT fif;
	FIBITMAP* fib;

	fif = FreeImage_GetFIFFromFilename(path.c_str());

	MF_ASSERT(!IsCompressed(image) && !IsFloat(image));
	MF_ASSERT(fif == FIF_BMP || fif == FIF_JPEG || fif == FIF_JP2 || fif == FIF_PNG || fif == FIF_TARGA);

	fib = FreeImage_Allocate(image.width, image.height, BPP(image.format));

	memcpy(FreeImage_GetBits(fib), image.data, Size(image));
	FreeImage_Save(fif, fib, path.c_str(), flags);

	FreeImage_Unload(fib);

	return true;
}


bool NImage::Save(const string& path, const SImage& image, bool swapChannels, bool invertY, int flags)
{
	SImage workingImage = Copy(image);

	if (swapChannels)
		SwapChannels(workingImage, 0, 2);

	if (invertY)
		SAFE_IMAGE_FUNCTION(workingImage, InvertY(workingImage));

	return Save(path, workingImage, flags);
}


bool NImage::SaveToMemory_JPEG50(const SImage& image, uint8*& data, int& dataSize)
{
	MF_ASSERT(!IsCompressed(image) && !IsFloat(image));

	FIBITMAP* fib = FreeImage_Allocate(image.width, image.height, BPP(image.format));
	memcpy(FreeImage_GetBits(fib), image.data, Size(image));

	FIMEMORY* fim = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_JPEG, fib, fim, JPEG_QUALITYNORMAL | JPEG_BASELINE | JPEG_SUBSAMPLING_444);
	FreeImage_AcquireMemory(fim, &data, (DWORD*)&dataSize);
	FreeImage_CloseMemory(fim);

	FreeImage_Unload(fib);

	return true;
}


bool NImage::IsCompressed(EFormat format)
{
	if (
		format == EFormat::DXT1 ||
		format == EFormat::DXT5)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool NImage::IsCompressed(const SImage& image)
{
	return IsCompressed(image.format);
}


bool NImage::IsFloat(EFormat format)
{
	if (
		format == EFormat::R32 ||
		format == EFormat::RG32 ||
		format == EFormat::RGB32 ||
		format == EFormat::RGBA32)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool NImage::IsFloat(const SImage& image)
{
	return IsFloat(image.format);
}


uint8 NImage::BPP(EFormat format)
{
	if (format == EFormat::R8)
	{
		return 8;
	}
	else if (format == EFormat::RG8)
	{
		return 16;
	}
	else if (format == EFormat::RGB8)
	{
		return 24;
	}
	else if (format == EFormat::RGBA8)
	{
		return 32;
	}
	else if (format == EFormat::R32)
	{
		return 32;
	}
	else if (format == EFormat::RG32)
	{
		return 64;
	}
	else if (format == EFormat::RGB32)
	{
		return 96;
	}
	else if (format == EFormat::RGBA32)
	{
		return 128;
	}
	else
	{
		return 0;
	}
}


uint8 NImage::ChannelsCount(EFormat format)
{
	if (format == EFormat::R8 || format == EFormat::R32)
	{
		return 1;
	}
	else if (format == EFormat::RG8 || format == EFormat::RG32)
	{
		return 2;
	}
	else if (format == EFormat::RGB8 || format == EFormat::RGB32)
	{
		return 3;
	}
	else if (format == EFormat::RGBA8 || format == EFormat::RGBA32)
	{
		return 4;
	}
	else
	{
		return 0;
	}
}


NImage::EFormat NImage::SingleChannelFormat(EFormat format)
{
	if (format == EFormat::RG8 || format == EFormat::RGB8 || format == EFormat::RGBA8)
		return EFormat::R8;
	else if (format == EFormat::RG32 || format == EFormat::RGB32 || format == EFormat::RGBA32)
		return EFormat::R32;
	else
		return EFormat::Unknown;
}


NImage::EFormat NImage::MultiChannelsFormat(EFormat singleChannelFormat, int channelsCount)
{
	if (singleChannelFormat == EFormat::R8)
	{
		if (channelsCount == 2) return EFormat::RG8;
		else if (channelsCount == 3) return EFormat::RGB8;
		else if (channelsCount == 4) return EFormat::RGBA8;
	}
	else if (singleChannelFormat == EFormat::R32)
	{
		if (channelsCount == 2) return EFormat::RG32;
		else if (channelsCount == 3) return EFormat::RGB32;
		else if (channelsCount == 4) return EFormat::RGBA32;
	}

	return EFormat::Unknown;
}


uint8 NImage::PixelSize(EFormat format)
{
	if (format == EFormat::R8)
	{
		return 1;
	}
	else if (format == EFormat::RG8)
	{
		return 2;
	}
	else if (format == EFormat::RGB8)
	{
		return 3;
	}
	else if (format == EFormat::RGBA8)
	{
		return 4;
	}
	else if (format == EFormat::R32)
	{
		return 4;
	}
	else if (format == EFormat::RG32)
	{
		return 8;
	}
	else if (format == EFormat::RGB32)
	{
		return 12;
	}
	else if (format == EFormat::RGBA32)
	{
		return 16;
	}
	else
	{
		return 0;
	}
}


uint8 NImage::PixelSize(const SImage& image)
{
	return PixelSize(image.format);
}


uint32 NImage::Size(int width, int height, EFormat format)
{
	if (format == EFormat::DXT1)
	{
		int blockSize = 8;
		int blocksCount = ( (width + 3)/4 ) * ( (height + 3)/4 );
		return blockSize * blocksCount;
	}
	else if (format == EFormat::DXT5)
	{
		int blockSize = 16;
		int blocksCount = ( (width + 3)/4 ) * ( (height + 3)/4 );
		return blockSize * blocksCount;
	}
	else
	{
		return PixelSize(format) * width * height;
	}
}


uint32 NImage::Size(const SImage& image)
{
	return Size(image.width, image.height, image.format);
}


uint8 NImage::MipmapsCount(int width, int height)
{
	uint8 mipmapsCount = 1;
	int size = Max(width, height);

	if (size == 1)
		return 1;

	do
	{
		size >>= 1;
		mipmapsCount++;
	}
	while (size != 1);

	return mipmapsCount;
}


void NImage::SwapChannels(SImage& image, int firstChannelIndex, int secondChannelIndex)
{
	MF_ASSERT(!IsCompressed(image));

	uint8 pixelSize = PixelSize(image);
	uint8 channelSize = pixelSize / ChannelsCount(image.format);
	uint32 pixelsCount = Size(image) / pixelSize;
	uint8* tempPixel = new uint8[pixelSize];

	for (uint32 i = 0; i < pixelsCount; i++)
	{
		memcpy(tempPixel, image.data + i*pixelSize, pixelSize);
		memcpy(image.data + i*pixelSize + firstChannelIndex*channelSize, tempPixel + secondChannelIndex*channelSize, channelSize);
		memcpy(image.data + i*pixelSize + secondChannelIndex*channelSize, tempPixel + firstChannelIndex*channelSize, channelSize);
	}

	delete[] tempPixel;
}


NImage::SImage NImage::Crop(const SImage& image, int cropX, int cropY, int cropWidth, int cropHeight)
{
	MF_ASSERT(!IsCompressed(image));

	uint8 pixelSize = PixelSize(image);
	SImage croppedImage = Create(cropWidth, cropHeight, image.format);

	for (uint16 j = 0; j < cropHeight; j++)
	{
		const uint8* srcData = &image.data[pixelSize * Idx(cropX, cropY + j, image.width)];
		uint8* dstData = &croppedImage.data[pixelSize * Idx(0, j, croppedImage.width)];

		memcpy(dstData, srcData, cropWidth * pixelSize);
	}

	return croppedImage;
}


NImage::SImage NImage::Shift(const SImage& image, int shiftX, int shiftY)
{
	MF_ASSERT(!IsCompressed(image));

	// normalize offsets

	shiftX = Wrap(shiftX, 0, image.width - 1);
	shiftY = Wrap(shiftY, 0, image.height - 1);

	// shift

	uint8 pixelSize = PixelSize(image);
	SImage shiftedImage = Create(image);

	for (uint16 j = 0; j < image.height; j++)
	{
		for (uint16 i = 0; i < image.width; i++)
		{
			int x = (i + shiftX) % image.width;
			int y = (j + shiftY) % image.height;

			const uint8* srcData = &image.data[pixelSize * Idx(i, j, image.width)];
			uint8* dstData = &shiftedImage.data[pixelSize * Idx(x, y, image.width)];

			memcpy(dstData, srcData, pixelSize);
		}
	}

	//

	return shiftedImage;
}


NImage::SImage NImage::InvertY(const SImage& image)
{
	MF_ASSERT(!IsCompressed(image));

	uint8 pixelSize = PixelSize(image);
	SImage invertedImage = Create(image);

	for (uint16 j = 0; j < image.height; j++)
	{
		const uint8* srcData = &image.data[pixelSize * Idx(0, j, image.width)];
		uint8* dstData = &invertedImage.data[pixelSize * Idx(0, image.height - j - 1, image.width)];

		memcpy(dstData, srcData, image.width * pixelSize);
	}

	return invertedImage;
}


NImage::SImage NImage::Scale(const SImage& image, int scaledWidth, int scaledHeight, EFilter filter)
{
	MF_ASSERT(!IsCompressed(image) && !IsFloat(image));

	SImage scaledImage = Create(scaledWidth, scaledHeight, image.format);

	FIBITMAP* fibInput = FreeImage_Allocate(image.width, image.height, BPP(image.format));
	memcpy(FreeImage_GetBits(fibInput), image.data, Size(image));

	FIBITMAP* fibOutput = FreeImage_Rescale(fibInput, scaledWidth, scaledHeight, (FREE_IMAGE_FILTER)filter);
	memcpy(scaledImage.data, FreeImage_GetBits(fibOutput), Size(scaledImage));

	FreeImage_Unload(fibInput);
	FreeImage_Unload(fibOutput);

	return scaledImage;
}


vector<NImage::SImage> NImage::GenerateMipmaps(const SImage& image, EFilter filter)
{
	vector<SImage> mipmaps;

	MF_ASSERT(!IsCompressed(image) && !IsFloat(image));
	MF_ASSERT(IsPowerOfTwo(image.width) && IsPowerOfTwo(image.height));

	uint8 mipmapsCount = MipmapsCount(image.width, image.height);
	mipmaps.resize(mipmapsCount);
	mipmaps[0] = Copy(image);

	int width = image.width;
	int height = image.height;

	for (uint8 i = 1; i < mipmapsCount; i++)
	{
		if (width > 1)
			width >>= 1;
		if (height > 1)
			height >>= 1;

		mipmaps[i] = Scale(mipmaps[i-1], width, height, filter);
	}

	return mipmaps;
}


void NImage::Saturate(SImage& image)
{
	MF_ASSERT(IsFloat(image));

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			float* data = (float*)image.data;
			int idx = Idx(x, y, image.width);
			int channelsCount = ChannelsCount(image.format);

			for (int i = 0; i < channelsCount; i++)
				data[idx*channelsCount + i] = NMath::Saturate(data[idx*channelsCount + i]);
		}
	}
}


void NImage::Difference(const SImage& image1, const SImage& image2, int offsetX, int offsetY, int width, int height, const SVector3& metric, float& total, float& maxPixel)
{
	MF_ASSERT(image1.width == image2.width);
	MF_ASSERT(image1.height == image2.height);
	MF_ASSERT(image1.format == image2.format);
	MF_ASSERT(image1.format == EFormat::R8 || image1.format == EFormat::RGB8);

	total = 0.0f;
	maxPixel = 0.0f;

	if (image1.format == EFormat::R8)
	{
		for (int y = offsetY; y < offsetY + height; y++)
		{
			for (int x = offsetX; x < offsetX + width; x++)
			{
				int index = Idx(x, y, image1.width);
				float f1 = IntToNormalizedFloat(image1.data[index]);
				float f2 = IntToNormalizedFloat(image2.data[index]);

				float difference = Sqr(f1 - f2);

				total += difference;
				maxPixel = Max(maxPixel, difference);
			}
		}
	}
	else if (image1.format == EFormat::RGB8)
	{
		for (int y = 0; y < image1.height; y++)
		{
			for (int x = 0; x < image1.width; x++)
			{
				int index = Idx(x, y, image1.width);
				SVector3 v1 = PixelToVector3(image1, index);
				SVector3 v2 = PixelToVector3(image2, index);

				float difference = DistanceSquared(metric*v1, metric*v2);

				total += difference;
				maxPixel = Max(maxPixel, difference);
			}
		}
	}
}


void NImage::Difference(const SImage& image1, const SImage& image2, const SVector3& metric, float& total, float& maxPixel)
{
	Difference(image1, image2, 0, 0, image1.width, image1.height, metric, total, maxPixel);
}


vector<NImage::SImage> NImage::SplitChannels(const SImage& image)
{
	int channelsCount = ChannelsCount(image.format);
	EFormat singleChannelFormat = SingleChannelFormat(image.format);
	int multiChannelsPixelSize = PixelSize(image.format);
	int singleChannelPixelSize = PixelSize(singleChannelFormat);

	vector<SImage> images;
	images.resize(channelsCount);
	for (int i = 0; i < channelsCount; i++)
		images[i] = Create(image.width, image.height, singleChannelFormat);

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			int index = Idx(x, y, image.width);

			for (int i = 0; i < channelsCount; i++)
			{
				memcpy(
					images[i].data + index*singleChannelPixelSize,
					image.data + index*multiChannelsPixelSize + i*singleChannelPixelSize,
					singleChannelPixelSize);
			}
		}
	}

	return images;
}


NImage::SImage NImage::MergeChannelImages(const vector<SImage>& images)
{
	int channelsCount = (int)images.size();
	EFormat multiChannelsFormat = MultiChannelsFormat(images[0].format, channelsCount);
	int singleChannelPixelSize = PixelSize(images[0].format);
	int multiChannelsPixelSize = PixelSize(multiChannelsFormat);

	SImage image = Create(images[0].width, images[0].height, multiChannelsFormat);

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			int index = Idx(x, y, image.width);

			for (int i = 0; i < channelsCount; i++)
			{
				memcpy(
					image.data + index*multiChannelsPixelSize + i*singleChannelPixelSize,
					images[i].data + index*singleChannelPixelSize,
					singleChannelPixelSize);
			}
		}
	}

	return image;
}


NImage::SImage NImage::Compress(const SImage& image, EFormat compressionFormat)
{
	MF_ASSERT(image.format == EFormat::RGBA8);
	MF_ASSERT(IsCompressed(compressionFormat));

	int flags = 0;
	if (compressionFormat == EFormat::DXT1)
		flags = squish::kDxt1;
	else if (compressionFormat == EFormat::DXT5)
		flags = squish::kDxt5;
	flags |= squish::kColourIterativeClusterFit;

	SImage compressedImage = Create(image.width, image.height, compressionFormat);
	squish::CompressImage(image.data, image.width, image.height, compressedImage.data, flags);

	return compressedImage;
}


NImage::SImage NImage::Decompress(const SImage& image)
{
	MF_ASSERT(IsCompressed(image));

	int flags = 0;
	if (image.format == EFormat::DXT1)
		flags = squish::kDxt1;
	else if (image.format == EFormat::DXT5)
		flags = squish::kDxt5;

	SImage decompressedImage = Create(image.width, image.height, EFormat::RGBA8);
	squish::DecompressImage(decompressedImage.data, image.width, image.height, image.data, flags);

	return decompressedImage;
}


NImage::SImage NImage::Checkerboard(const SImage& image, bool flip)
{
	SImage checkerImage = Create(image.width / 2, image.height, image.format);
	uint8 pixelSize = PixelSize(image);

	for (int y = 0; y < checkerImage.height; y++)
	{
		for (int x = 0; x < checkerImage.width; x++)
		{
			uint8* srcData;
			uint8* dstData = &checkerImage.data[pixelSize * Idx(x, y, checkerImage.width)];

			if (!flip)
			{
				if (y % 2 == 0)
					srcData = &image.data[pixelSize * Idx(2*x, y, image.width)];
				else
					srcData = &image.data[pixelSize * Idx(2*x + 1, y, image.width)];
			}
			else
			{
				if (y % 2 == 0)
					srcData = &image.data[pixelSize * Idx(2*x + 1, y, image.width)];
				else
					srcData = &image.data[pixelSize * Idx(2*x, y, image.width)];
			}

			memcpy(dstData, srcData, pixelSize);
		}
	}

	return checkerImage;
}


NImage::SImage NImage::Decheckerboard(const SImage& image, bool flipped)
{
	SImage decheckerImage = Create(image.width * 2, image.height, image.format);
	uint8 pixelSize = PixelSize(image);

	memset(decheckerImage.data, 0, Size(decheckerImage));

	for (int y = 0; y < decheckerImage.height; y++)
	{
		for (int x = 0; x < decheckerImage.width; x++)
		{
			uint8* srcData;
			uint8* dstData = &decheckerImage.data[pixelSize * Idx(x, y, decheckerImage.width)];

			if (!flipped)
			{
				if (x % 2 == 0 && y % 2 == 0)
					srcData = &image.data[pixelSize * Idx(x/2, y, image.width)];
				else if (x % 2 == 1 && y % 2 == 1)
					srcData = &image.data[pixelSize * Idx((x - 1)/2, y, image.width)];
			}
			else
			{
				if (x % 2 == 1 && y % 2 == 0)
					srcData = &image.data[pixelSize * Idx((x - 1)/2, y, image.width)];
				else if (x % 2 == 0 && y % 2 == 1)
					srcData = &image.data[pixelSize * Idx(x/2, y, image.width)];
			}

			memcpy(dstData, srcData, pixelSize);
		}
	}

	return decheckerImage;
}


void NImage::DecheckerboardFill(SImage& image, bool flipped)
{
	MF_ASSERT(image.format == EFormat::RGB8);

	uint8 pixelSize = PixelSize(image);

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			bool fill =
				( !flipped && ((x % 2 == 1 && y % 2 == 0) || (x % 2 == 0 && y % 2 == 1)) ) ||
				( flipped && ((x % 2 == 0 && y % 2 == 0) || (x % 2 == 1 && y % 2 == 1)) );

			if (fill)
			{
				int index0 = Idx(x, y, image.width);
				int index1 = Idx(Clamp(x - 1, 0, image.width - 1), Clamp(y + 0, 0, image.height - 1), image.width);
				int index2 = Idx(Clamp(x + 1, 0, image.width - 1), Clamp(y + 0, 0, image.height - 1), image.width);
				int index3 = Idx(Clamp(x + 0, 0, image.width - 1), Clamp(y - 1, 0, image.height - 1), image.width);
				int index4 = Idx(Clamp(x + 0, 0, image.width - 1), Clamp(y + 1, 0, image.height - 1), image.width);

				SVector3 pixel = WeightedAverage(image, index1, 0.25f, index2, 0.25f, index3, 0.25f, index4, 0.25f);
				SVector3ui24 pixel_uint24 = Vector3ToSVector3ui24_Inclusive(pixel);
				memcpy(&image.data[index0 * pixelSize], &pixel_uint24, pixelSize);
			}
		}
	}
}


NImage::SImage NImage::DecheckerboardMerge(const SImage& image, const SImage& flippedImage)
{
	MF_ASSERT(image.width == flippedImage.width && image.height == flippedImage.height && image.format == flippedImage.format);

	SImage mergedImage = Create(image);
	uint8 pixelSize = PixelSize(mergedImage);

	for (int y = 0; y < mergedImage.height; y++)
	{
		for (int x = 0; x < mergedImage.width; x++)
		{
			uint8* srcData;
			uint8* dstData = &mergedImage.data[pixelSize * Idx(x, y, mergedImage.width)];

			if ( (x % 2 == 0 && y % 2 == 0) || (x % 2 == 1 && y % 2 == 1) )
				srcData = &image.data[pixelSize * Idx(x, y, mergedImage.width)];
			else
				srcData = &flippedImage.data[pixelSize * Idx(x, y, mergedImage.width)];

			memcpy(dstData, srcData, pixelSize);
		}
	}

	return mergedImage;
}


void NImage::AddIn(SImage& image, float value)
{
	MF_ASSERT(IsFloat(image));

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			float* data = (float*)image.data;
			int idx = Idx(x, y, image.width);
			int channelsCount = ChannelsCount(image.format);

			for (int i = 0; i < channelsCount; i++)
				data[idx*channelsCount + i] += value;
		}
	}
}


void NImage::MulIn(SImage& image, float value)
{
	MF_ASSERT(IsFloat(image));

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			float* data = (float*)image.data;
			int idx = Idx(x, y, image.width);
			int channelsCount = ChannelsCount(image.format);

			for (int i = 0; i < channelsCount; i++)
				data[idx*channelsCount + i] *= value;
		}
	}
}


void NImage::PowIn(SImage& image, float value)
{
	MF_ASSERT(IsFloat(image));

	for (int y = 0; y < image.height; y++)
	{
		for (int x = 0; x < image.width; x++)
		{
			float* data = (float*)image.data;
			int idx = Idx(x, y, image.width);
			int channelsCount = ChannelsCount(image.format);

			for (int i = 0; i < channelsCount; i++)
				data[idx*channelsCount + i] = Pow(data[idx*channelsCount + i], value);
		}
	}
}
