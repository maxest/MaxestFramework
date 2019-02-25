#include "neural_network.h"
#include "../math/common.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NCommon;


bool NNeuralNetwork::SModel::Create(const string& path)
{
	Destroy();

	string line;
	NSystem::CFile file;
	if (file.Open(path, NSystem::CFile::EOpenMode::ReadText))
	{
		file.ReadLine(line);
		int layersCount = NEssentials::ToInt(line);

		for (int i = 0; i < layersCount; i++)
		{
			file.ReadLine(line);

			if (line == "ReLU")
			{
				SReLULayer* layer = new SReLULayer();
				layers.push_back((SLayer*)layer);
			}
			else if (line == "Sigmoid")
			{
				SSigmoidLayer* layer = new SSigmoidLayer();
				layers.push_back((SLayer*)layer);
			}
			else if (line == "Linear")
			{
				SLinearLayer* layer = new SLinearLayer();
				layer->Load(file);
				layers.push_back((SLayer*)layer);
			}
			else if (line == "Conv2D")
			{
				SConv2DLayer* layer = new SConv2DLayer();
				layer->Load(file);
				layers.push_back((SLayer*)layer);
			}
			else if (line == "BatchNorm2D")
			{
				SBatchNorm2DLayer* layer = new SBatchNorm2DLayer();
				layer->Load(file);
				layers.push_back((SLayer*)layer);
			}
			else
			{
				MF_ASSERT(false);
			}
		}

		file.Close();

		return true;
	}
	else
	{
		return false;
	}
}


void NNeuralNetwork::SModel::Destroy()
{
	for (uint i = 0; i < layers.size(); i++)
		delete layers[i];
	layers.clear();
}


NNeuralNetwork::STensor NNeuralNetwork::SModel::Eval(const STensor& input)
{
	vector<STensor> tempTensors;
	tempTensors.push_back(input);

	for (uint i = 0; i < layers.size(); i++)
	{
		if (layers[i]->type == SLayer::Type::ReLU)
		{
			ReLU(tempTensors.back());
		}
		else if (layers[i]->type == SLayer::Type::Sigmoid)
		{
			Sigmoid(tempTensors.back());
		}
		else if (layers[i]->type == SLayer::Type::Linear)
		{
			SLinearLayer* layer = (SLinearLayer*)layers[i];
			STensor output = Linear(*layer, tempTensors.back());
			tempTensors.push_back(output);
		}
		else if (layers[i]->type == SLayer::Type::Conv2D)
		{
			SConv2DLayer* layer = (SConv2DLayer*)layers[i];
			STensor output = Conv2D(*layer, tempTensors.back());
			tempTensors.push_back(output);
		}
		else if (layers[i]->type == SLayer::Type::BatchNorm2D)
		{
			SBatchNorm2DLayer* layer = (SBatchNorm2DLayer*)layers[i];
			BatchNorm2D(*layer, tempTensors.back());
		}
		else
		{
			MF_ASSERT(false);
		}
	}

	// starts with 1 because it contains input; doesn't include last because that is where the result is
	for (uint i = 1; i < tempTensors.size() - 1; i++)
		tempTensors[i].Destroy();

	return tempTensors.back();
}


void NNeuralNetwork::AddIn(STensor& tensor1, const STensor& tensor2)
{
	MF_ASSERT(tensor1.Size() == tensor2.Size());

	for (int i = 0; i < tensor1.Size(); i++)
		tensor1.data[i] += tensor2.data[i];
}


void NNeuralNetwork::ReLU(STensor& tensor)
{
	for (int i = 0; i < tensor.Size(); i++)
	{
		tensor.data[i] = NEssentials::Max(tensor.data[i], 0.0);
	}
}


void NNeuralNetwork::Sigmoid(STensor& tensor)
{
	for (int i = 0; i < tensor.Size(); i++)
	{
		tensor.data[i] = 1.0 / (1.0 + NMath::Exp(-tensor.data[i]));
	}
}


NNeuralNetwork::STensor NNeuralNetwork::Linear(const SLinearLayer& layer, STensor& input)
{
	MF_ASSERT(layer.inFeaturesCount == input.Size());

	STensor output;
	output.Create(layer.outFeaturesCount);

	for (int j = 0; j < layer.outFeaturesCount; j++)
	{
		double newOutput = layer.Bias(j);
		for (int i = 0; i < layer.inFeaturesCount; i++)
			newOutput += input(i) * layer.Weight(i, j);
		output(j) = newOutput;
	}

	return output;
}


NNeuralNetwork::STensor NNeuralNetwork::Conv2D(const SConv2DLayer& layer, STensor& inChannels)
{
	struct SUtils
	{
		double Conv2D(const SConv2DLayer& layer, int outChannelIndex, int j, int i, STensor& inChannels)
		{
			MF_ASSERT(layer.inChannelsCount == inChannels.Size(0));

			int halfKernelX = (layer.kernelX - 1) / 2;
			int halfKernelY = (layer.kernelY - 1) / 2;

			double output = layer.Bias(outChannelIndex);

			for (int k = 0; k < layer.inChannelsCount; k++)
			{
				for (int v = 0; v < layer.kernelY; v++)
				{
					for (int u = 0; u < layer.kernelX; u++)
					{
						double x = 0.0f;
						int px = i + u - halfKernelX;
						int py = j + v - halfKernelY;
						if (px >= 0 && px < inChannels.Size(2) && py >= 0 && py < inChannels.Size(1))
							x = inChannels(k, py, px);

						double w = layer.Weight(k, outChannelIndex, u, v);

						output += x * w;
					}
				}
			}

			return output;
		}
	} utils;

	STensor outChannels;
	outChannels.Create(layer.outChannelsCount, inChannels.Size(1), inChannels.Size(2));

	for (int k = 0; k < layer.outChannelsCount; k++)
	{
		for (int j = 0; j < inChannels.Size(1); j++)
		{
			for (int i = 0; i < inChannels.Size(2); i++)
			{
				double value = utils.Conv2D(layer, k, j, i, inChannels);
				outChannels(k, j, i) = value;
			}
		}
	}

	return outChannels;
}


void NNeuralNetwork::BatchNorm2D(const SBatchNorm2DLayer& layer, STensor& tensor)
{
	MF_ASSERT(layer.featuresCount == tensor.Size(0));

	for (int k = 0; k < tensor.Size(0); k++)
	{
		for (int j = 0; j < tensor.Size(1); j++)
		{
			for (int i = 0; i < tensor.Size(2); i++)
			{
				double value = tensor(k, j, i);
				value = (value - layer.Mean(k)) / NMath::Sqrt(layer.Var(k) + NMath::cEpsilon5d);
				value = layer.Weight(k)*value + layer.Bias(k);
				tensor(k, j, i) = value;
			}
		}
	}
}


#ifdef MAXEST_FRAMEWORK_DESKTOP
	void NNeuralNetwork::ImageToTensors(const NImage::SImage& image, STensor& red, STensor& green, STensor& blue)
	{
		MF_ASSERT(image.format == NImage::EFormat::RGB8);

		red.Create(1, image.width * image.height);
		green.Create(1, image.width * image.height);
		blue.Create(1, image.width * image.height);

		for (int i = 0; i < image.width * image.height; i++)
		{
			red(0, i) = (double)image.data[3 * i + 0] / 255.0;
			green(0, i) = (double)image.data[3 * i + 1] / 255.0;
			blue(0, i) = (double)image.data[3 * i + 2] / 255.0;
		}
	}


	NImage::SImage NNeuralNetwork::TensorsToImage(int width, int height, STensor& red, STensor& green, STensor& blue)
	{
		NImage::SImage image = Create(width, height, NImage::EFormat::RGB8);

		for (int i = 0; i < width * height; i++)
		{
			uint8 r = (uint8)(255.0 * Saturate(red(0, i)));
			uint8 g = (uint8)(255.0 * Saturate(green(0, i)));
			uint8 b = (uint8)(255.0 * Saturate(blue(0, i)));

			image.data[3 * i + 0] = r;
			image.data[3 * i + 1] = g;
			image.data[3 * i + 2] = b;
		}

		return image;
	}


	void NNeuralNetwork::ProcessImage(const string& residualModelPath, const string& inputImagePath, const string& outputImagePath)
	{
		SModel model;
		MF_ASSERT(model.Create(residualModelPath));

		NImage::SImage inputImage;
		MF_ASSERT(NImage::Load(inputImagePath, inputImage, true, true, NImage::EFormat::RGB8));
		STensor red;
		STensor green;
		STensor blue;
		ImageToTensors(inputImage, red, green, blue);
		red.Reshape(1, inputImage.height, inputImage.width);
		green.Reshape(1, inputImage.height, inputImage.width);
		blue.Reshape(1, inputImage.height, inputImage.width);

		uint64 bef = NSystem::TickCount();
		STensor redDiff = model.Eval(red);
		STensor greenDiff = model.Eval(green);
		STensor blueDiff = model.Eval(blue);
		uint64 aft = NSystem::TickCount();
		cout << 0.000001 * (aft - bef) << endl;

		red.Reshape(1, inputImage.width * inputImage.height);
		green.Reshape(1, inputImage.width * inputImage.height);
		blue.Reshape(1, inputImage.width * inputImage.height);
		AddIn(red, redDiff);
		AddIn(green, greenDiff);
		AddIn(blue, blueDiff);
		NImage::SImage outputImage = TensorsToImage(inputImage.width, inputImage.height, red, green, blue);
		NImage::Save(outputImagePath, outputImage, true, true);

		model.Destroy();
		NImage::Destroy(inputImage);
		NImage::Destroy(outputImage);
		red.Destroy();
		green.Destroy();
		blue.Destroy();
		redDiff.Destroy();
		greenDiff.Destroy();
		blueDiff.Destroy();
	}
#endif


void NNeuralNetwork::UnitTest(const string& path)
{
	SModel model;
	MF_ASSERT(model.Create(path));

	STensor x;
	for (double value = 0.0; value <= 0.9; value += 0.1)
	{
		x.Create(1);
		x(0) = value;

		STensor y = model.Eval(x);
		cout << y(0) << endl;

		x.Destroy();
		y.Destroy();
	}

	model.Destroy();
}
