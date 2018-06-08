#pragma once


#include "../essentials/main.h"
#include "../system/main.h"
#include "../image/image.h"


namespace NMaxestFramework { namespace NCommon { namespace NNeuralNetwork
{
	struct STensor
	{
		double* data;
		vector<int> shape;

		STensor()
		{
			data = nullptr;
		}

		int Size() const
		{
			MF_ASSERT(shape.size() > 0);

			int size = shape[0];
			for (uint i = 1; i < shape.size(); i++)
				size *= shape[i];

			return size;
		}
		int Size(int dim) const
		{
			MF_ASSERT(dim < shape.size());

			return shape[dim];
		}

		void Create(int dim)
		{
			Destroy();

			data = new double[dim];
			shape.resize(1);
			shape[0] = dim;
		}
		void Create(int dim1, int dim2)
		{
			Create(dim1 * dim2);
			Reshape(dim1, dim2);
		}
		void Create(int dim1, int dim2, int dim3)
		{
			Create(dim1 * dim2 * dim3);
			Reshape(dim1, dim2, dim3);
		}

		void Destroy()
		{
			SAFE_DELETE_ARRAY(data);
			shape.clear();
		}

		void Reshape(int dim1, int dim2)
		{
			MF_ASSERT(dim1 * dim2 == Size());

			shape.resize(2);
			shape[0] = dim1;
			shape[1] = dim2;
		}
		void Reshape(int dim1, int dim2, int dim3)
		{
			MF_ASSERT(dim1 * dim2 * dim3 == Size());

			shape.resize(3);
			shape[0] = dim1;
			shape[1] = dim2;
			shape[2] = dim3;
		}

		int Idx(int dim) const
		{
			MF_ASSERT(shape.size() == 1);

			return dim;
		}
		int Idx(int dim1, int dim2) const
		{
			MF_ASSERT(shape.size() == 2);

			return dim1*shape[1] + dim2;
		}
		int Idx(int dim1, int dim2, int dim3) const
		{
			MF_ASSERT(shape.size() == 3);

			return shape[2] * (dim1*shape[1] + dim2) + dim3;
		}

		double& operator()(int dim)
		{
			return data[Idx(dim)];
		}
		double& operator()(int dim1, int dim2)
		{
			return data[Idx(dim1, dim2)];
		}
		double& operator()(int dim1, int dim2, int dim3)
		{
			return data[Idx(dim1, dim2, dim3)];
		}
	};

	struct SLayer
	{
		enum class Type { None, ReLU, Sigmoid, Linear, Conv2D, BatchNorm2D };

		Type type;

		SLayer()
		{
			type = Type::None;
		}
	};

	struct SReLULayer : public SLayer
	{
		SReLULayer()
		{
			type = Type::ReLU;
		}
	};

	struct SSigmoidLayer : public SLayer
	{
		SSigmoidLayer()
		{
			type = Type::Sigmoid;
		}
	};

	struct SLinearLayer : public SLayer
	{
		int inFeaturesCount;
		int outFeaturesCount;
		bool hasBias;

		vector<double> weights;
		vector<double> biases;

		SLinearLayer()
		{
			type = Type::Linear;
		}

		void Load(NSystem::CFile& file)
		{
			string line;

			file.ReadLine(line);
			inFeaturesCount = NEssentials::ToInt(line);
			file.ReadLine(line);
			outFeaturesCount = NEssentials::ToInt(line);
			file.ReadLine(line);
			if (line == "bias_yes")
				hasBias = true;
			else
				hasBias = false;

			int weightsCount = inFeaturesCount * outFeaturesCount;
			int biasesCount = outFeaturesCount;
			weights.resize(weightsCount);
			biases.resize(hasBias ? biasesCount : 0);
			for (int i = 0; i < weightsCount; i++)
			{
				file.ReadLine(line);
				weights[i] = NEssentials::ToDouble(line);
			}
			if (hasBias)
			{
				for (int i = 0; i < biasesCount; i++)
				{
					file.ReadLine(line);
					biases[i] = NEssentials::ToDouble(line);
				}
			}
		}

		double Weight(int inFeatureIndex, int outFeatureIndex) const
		{
			int weightIndex = NEssentials::Idx(inFeatureIndex, outFeatureIndex, inFeaturesCount);
			return weights[weightIndex];
		}

		double Bias(int outFeatureIndex) const
		{
			return hasBias ? biases[outFeatureIndex] : 0.0;
		}
	};

	struct SConv2DLayer : public SLayer
	{
		int inChannelsCount;
		int outChannelsCount;
		int kernelX, kernelY;

		vector<double> weights;
		vector<double> biases;

		SConv2DLayer()
		{
			type = Type::Conv2D;
		}

		void Load(NSystem::CFile& file)
		{
			string line;

			file.ReadLine(line);
			inChannelsCount = NEssentials::ToInt(line);
			file.ReadLine(line);
			outChannelsCount = NEssentials::ToInt(line);
			file.ReadLine(line);
			kernelX = NEssentials::ToInt(line);
			file.ReadLine(line);
			kernelY = NEssentials::ToInt(line);

			int weightsCount = inChannelsCount * outChannelsCount * kernelX * kernelY;
			int biasesCount = outChannelsCount;
			weights.resize(weightsCount);
			biases.resize(biasesCount);
			for (int i = 0; i < weightsCount; i++)
			{
				file.ReadLine(line);
				weights[i] = NEssentials::ToDouble(line);
			}
			for (int i = 0; i < biasesCount; i++)
			{
				file.ReadLine(line);
				biases[i] = NEssentials::ToDouble(line);
			}
		}

		double Weight(int inChannelIndex, int outChannelIndex, int u, int v) const
		{
			int channelIdx = NEssentials::Idx(inChannelIndex, outChannelIndex, inChannelsCount);
			int kernelIdx = NEssentials::Idx(u, v, kernelX);
			int weightMemOffset = (kernelX * kernelY) * channelIdx + kernelIdx;

			return weights[weightMemOffset];
		}

		double Bias(int outChannelIndex) const
		{
			return biases[outChannelIndex];
		}
	};

	struct SBatchNorm2DLayer : public SLayer
	{
		int featuresCount;

		vector<double> weights;
		vector<double> biases;
		vector<double> means;
		vector<double> vars;

		SBatchNorm2DLayer()
		{
			type = Type::BatchNorm2D;
		}

		void Load(NSystem::CFile& file)
		{
			string line;

			file.ReadLine(line);
			featuresCount = NEssentials::ToInt(line);

			weights.resize(featuresCount);
			biases.resize(featuresCount);
			means.resize(featuresCount);
			vars.resize(featuresCount);
			for (int i = 0; i < featuresCount; i++)
			{
				file.ReadLine(line);
				weights[i] = NEssentials::ToDouble(line);
			}
			for (int i = 0; i < featuresCount; i++)
			{
				file.ReadLine(line);
				biases[i] = NEssentials::ToDouble(line);
			}
			for (int i = 0; i < featuresCount; i++)
			{
				file.ReadLine(line);
				means[i] = NEssentials::ToDouble(line);
			}
			for (int i = 0; i < featuresCount; i++)
			{
				file.ReadLine(line);
				vars[i] = NEssentials::ToDouble(line);
			}
		}

		double Weight(int featureIndex) const
		{
			return weights[featureIndex];
		}

		double Bias(int featureIndex) const
		{
			return biases[featureIndex];
		}

		double Mean(int featureIndex) const
		{
			return means[featureIndex];
		}

		double Var(int featureIndex) const
		{
			return vars[featureIndex];
		}
	};

	struct SModel
	{
		vector<SLayer*> layers;

		bool Create(const string& path);
		void Destroy();

		STensor Eval(const STensor& input);
	};

	void AddIn(STensor& tensor1, const STensor& tensor2);

	void ReLU(STensor& tensor);
	void Sigmoid(STensor& tensor);
	STensor Linear(const SLinearLayer& layer, STensor& input);
	STensor Conv2D(const SConv2DLayer& layer, STensor& inChannels);
	void BatchNorm2D(const SBatchNorm2DLayer& layer, STensor& tensor);

	void ImageToTensors(const NImage::SImage& image, STensor& red, STensor& green, STensor& blue);
	NImage::SImage TensorsToImage(int width, int height, STensor& red, STensor& green, STensor& blue);
	void ProcessImage(const string& residualModelPath, const string& inputImagePath, const string& outputImagePath);

	void UnitTest(const string& path);
} } }
