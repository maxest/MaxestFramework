#include "../../../src/essentials/main.h"
#include "../../../src/math/main.h"
#include "../../../src/image/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;
using namespace NImage;


void Log(const string& msg)
{
	cout << msg << endl;
}


void Fourier(const string& imageFileName, const string& filterFileName, bool invertFilterValues, float filterMinValue, float filterMaxValue)
{
	// load

	SImage inputImage;
	NImage::Load(imageFileName, inputImage);
	if (inputImage.format != NImage::EFormat::R8)
	{
		Log("ERROR: Only grayscale input images are allowed");
		return;
	}

	SImage filterImage;
	NImage::Load(filterFileName, filterImage);
	if (inputImage.width != filterImage.width || inputImage.height != filterImage.height)
	{
		Log("ERROR: Image and filter sizes do not match");
		return;
	}
	if (inputImage.format != NImage::EFormat::R8)
	{
		Log("ERROR: Only grayscale filter images are allowed");
		return;
	}

	//

	int width = inputImage.width;
	int height = inputImage.height;

	// fourier

	SImage dftImage = DiscreteFourierTransform(inputImage);
	SImage dftShiftedImage = NImage::Shift(dftImage, width / 2, height / 2);

	// apply filter
	if (filterFileName != "")
	{
		// get a and b coefficients of "y = ax + b" rescale function
		SVector2 filterMinMaxRescale_lineCoefficients = SolveLineCoeffs(VectorCustom(0.0f, filterMinValue), VectorCustom(1.0f, filterMaxValue));

		for (uint16 j = 0; j < height; j++)
		{
			for (uint16 i = 0; i < width; i++)
			{
				if (invertFilterValues)
					filterImage.data[Idx(i, j, width)] = 255 - filterImage.data[Idx(i, j, width)];
				float filter = (float)filterImage.data[Idx(i, j, width)] / 255.0f;
				filter = filterMinMaxRescale_lineCoefficients.x * filter + filterMinMaxRescale_lineCoefficients.y;

				((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 0] *= filter;
				((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 1] *= filter;
			}
		}
	}
	delete[] dftImage.data;
	dftImage = NImage::Shift(dftShiftedImage, -width / 2, -height / 2);

	SImage idftImage = InverseDiscreteFourierTransform(dftImage);

	// generate output images

	SImage amplitudesImage = Create(width, height, NImage::EFormat::R8);
	SImage phasesImage = Create(width, height, NImage::EFormat::R8);
	SImage idftRealImage = Create(width, height, NImage::EFormat::R8);

	for (uint16 j = 0; j < height; j++)
	{
		for (uint16 i = 0; i < width; i++)
		{
			float amplitude = Sqrt(Sqr(((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 0]) + Sqr(((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 1]));
			float phase = ATan2(((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 1], ((float*)(dftShiftedImage.data))[2*Idx(i, j, width) + 0]);

			amplitude *= 0.01f;
			phase *= 100.0f;
			phase = fabs(phase);

			amplitudesImage.data[Idx(i, j, width)] = (uint8)Clamp(amplitude, 0.0f, 255.0f);
			phasesImage.data[Idx(i, j, width)] = (uint8)Clamp(phase, 0.0f, 255.0f);
			idftRealImage.data[Idx(i, j, width)] = (uint8)Clamp(((float*)(idftImage.data))[2*Idx(i, j, width) + 0], 0.0f, 255.0f);
		}
	}

	// save

	NImage::Save(imageFileName + string("__amplitudes.png"), amplitudesImage, false);
	NImage::Save(imageFileName + string("__phases.png"), phasesImage, false);
	NImage::Save(imageFileName + string("__idft_real.png"), idftRealImage, false);

	// clean

	delete[] dftImage.data;
	delete[] dftShiftedImage.data;
	delete[] idftImage.data;
	delete[] amplitudesImage.data;
	delete[] phasesImage.data;
	delete[] idftRealImage.data;
}


int main(int argc, const char* argv[])
{
	if (argc != 6)
	{
		Log("ERROR: Wrong number of input arguments given");
		return 1;
	}

	string imageFileName = string(argv[1]);
	string filterFileName = string(argv[2]);
	bool invertFilterValues = atoi(argv[3]) > 0 ? true : false;
	float filterMinValue = (float)atof(argv[4]);
	float filterMaxValue = (float)atof(argv[5]);

	NImage::Initialize();

	Fourier(imageFileName, filterFileName, invertFilterValues, filterMinValue, filterMaxValue);

	NImage::Deinitialize();

	return 0;
}
