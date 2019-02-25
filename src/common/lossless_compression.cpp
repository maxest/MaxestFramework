#include "lossless_compression.h"
#include "../math/main.h"


using namespace NMaxestFramework;
using namespace NEssentials;
using namespace NMath;


map<uint32, SDynamicBitArray> NCommon::Huffman(const vector<uint32>& pdf)
{
	struct SNode
	{
		bool isRoot;
		int symbol;
		int p;
		int leftChildIndex;
		int rightChildIndex;
	};

	struct SUtils
	{
		void Traverse_Recursive(const vector<SNode>& nodes, int nodeIndex, map<uint32, SDynamicBitArray>& symbolToCodeMap, SDynamicBitArray& runningDBA)
		{
			if (nodes[nodeIndex].symbol != cIntMax)
			{
				symbolToCodeMap[nodes[nodeIndex].symbol] = runningDBA;
			}
			else
			{
				SDynamicBitArray leftRunningDBA = runningDBA;
				SDynamicBitArray rightRunningDBA = runningDBA;
				leftRunningDBA.Add(0);
				rightRunningDBA.Add(1);

				Traverse_Recursive(nodes, nodes[nodeIndex].leftChildIndex, symbolToCodeMap, leftRunningDBA);
				Traverse_Recursive(nodes, nodes[nodeIndex].rightChildIndex, symbolToCodeMap, rightRunningDBA);
			}
		}
	} utils;

	MF_ASSERT(pdf.size() > 1);

	vector<SNode> nodes;

	for (uint32 i = 0; i < pdf.size(); i++)
	{
		SNode node;
		node.isRoot = true;
		node.symbol = i;
		node.p = pdf[i];
		node.leftChildIndex = -1;
		node.rightChildIndex = -1;

		nodes.push_back(node);
	}

	for (;;)
	{
		int lowestIndex1 = -1;
		int lowestP1 = cIntMax;
		for (uint i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].isRoot)
			{
				if (nodes[i].p < lowestP1)
				{
					lowestIndex1 = i;
					lowestP1 = nodes[i].p;
				}
			}
		}

		int lowestIndex2 = -1;
		int lowestP2 = cIntMax;
		for (int i = 0; i < (int)nodes.size(); i++)
		{
			if (nodes[i].isRoot)
			{
				if (nodes[i].p < lowestP2 && i != lowestIndex1)
				{
					lowestIndex2 = i;
					lowestP2 = nodes[i].p;
				}
			}
		}

		if (lowestIndex2 == -1)
		{
			break;
		}
		else
		{
			nodes[lowestIndex1].isRoot = false;
			nodes[lowestIndex2].isRoot = false;

			SNode node;
			node.isRoot = true;
			node.symbol = cIntMax;
			node.p = nodes[lowestIndex1].p + nodes[lowestIndex2].p;
			node.leftChildIndex = lowestIndex1;
			node.rightChildIndex = lowestIndex2;

			nodes.push_back(node);
		}
	}

	map<uint32, SDynamicBitArray> symbolToCodeMap;
	SDynamicBitArray runningDBA;
	utils.Traverse_Recursive(nodes, (int)nodes.size() - 1, symbolToCodeMap, runningDBA);

	return symbolToCodeMap;
}


void NCommon::CANSDistribution::SetPDF(const vector<uint32>& pdf, uint64 pdfSum)
{
	MF_ASSERT(IsPowerOfTwo(pdfSum) == true);

	_pdf = PDFNormalized(pdf, pdfSum);
	PDFForceNonZero(_pdf, true);

	_cdf_exclusive = CDF(_pdf, false);

	_pdfSum = pdfSum;
	_pdfSumLog2 = (int)Log2((double)_pdfSum);
}


void NCommon::CANSDistribution::SetPDF32(const vector<uint32>& pdf)
{
	SetPDF(pdf, (uint64)0xFFFF + 1);
}


void NCommon::CANSDistribution::SetPDF64(const vector<uint32>& pdf)
{
	SetPDF(pdf, (uint64)0xFFFFFFFF + 1);
}


uint32 NCommon::CANSDistribution::Symbol(uint64 x) const
{
	x = x % _pdfSum;

	for (uint32 i = 0; i < (uint32)_cdf_exclusive.size(); i++)
	{
		uint32 index = (uint32)_cdf_exclusive.size() - 1 - i;

		if (x >= _cdf_exclusive[index])
			return index;
	}

	MF_ASSERT(false);

	return 0;
}


void NCommon::CANSCoder32::Encode(const CANSDistribution& d, uint32 symbol)
{
	uint32 maxX = (uint32)(d._pdf[symbol] << (32 - d._pdfSumLog2));

	if (_x >= maxX)
	{
		_words.push_back(_x & 0xFFFF);
		_x >>= 16;
	}

	_x = ( (uint32)(_x / d._pdf[symbol]) << d._pdfSumLog2 ) + (_x % d._pdf[symbol]) + (uint32)d._cdf_exclusive[symbol];
}


uint32 NCommon::CANSCoder32::Decode(const CANSDistribution& d)
{
	int s = d.Symbol(_x);
	_x = d._pdf[s] * (_x >> d._pdfSumLog2) + (_x % d._pdfSum) - (uint32)d._cdf_exclusive[s];

	if (_x <= 0xFFFF)
	{
		if (_words.size() > 0)
		{
			_x = (_x << 16) + _words.back();
			_words.pop_back();
		}
	}

	return s;
}


void NCommon::CANSCoder32::WriteRaw(uint16 rawValue, uint8 bitsCount)
{
	MF_ASSERT(bitsCount > 0);

	uint32 pdf = 1 << (16 - bitsCount);
	uint32 maxX = ((uint32)1 << 16) * pdf;

	if (_x >= maxX)
	{
		_words.push_back(_x & 0xFFFF);
		_x >>= 16;
	}

	_x = (_x << bitsCount) | rawValue;
}


uint16 NCommon::CANSCoder32::ReadRaw(uint8 bitsCount)
{
	MF_ASSERT(bitsCount > 0);

	uint32 value = _x & NBit::Mask64(bitsCount);
	_x >>= bitsCount;

	if (_x <= 0xFFFF)
	{
		if (_words.size() > 0)
		{
			_x = (_x << 16) | _words.back();
			_words.pop_back();
		}
	}

	return value;
}


void NCommon::CANSCoder32::Reset()
{
	_x = 0;
	_words.clear();
}


void NCommon::CANSCoder64::Encode(const CANSDistribution& d, uint32 symbol)
{
	uint64 maxX = (uint64)d._pdf[symbol] << (64 - d._pdfSumLog2);

	if (_x >= maxX)
	{
		_words.push_back(_x & 0xFFFFFFFF);
		_x >>= 32;
	}

	_x = ( (uint64)(_x / d._pdf[symbol]) << d._pdfSumLog2 ) + (_x % d._pdf[symbol]) + d._cdf_exclusive[symbol];
}


uint32 NCommon::CANSCoder64::Decode(const CANSDistribution& d)
{
	int s = d.Symbol(_x);
	_x = d._pdf[s] * (_x >> d._pdfSumLog2) + (_x % d._pdfSum) - d._cdf_exclusive[s];

	if (_x <= 0xFFFFFFFF)
	{
		if (_words.size() > 0)
		{
			_x = (_x << 32) + _words.back();
			_words.pop_back();
		}
	}

	return s;
}


void NCommon::CANSCoder64::WriteRaw(uint32 rawValue, uint8 bitsCount)
{
	MF_ASSERT(bitsCount > 0);

	uint32 pdf = 1 << (32 - bitsCount);
	uint64 maxX = ((uint64)1 << 32) * pdf;

	if (_x >= maxX)
	{
		_words.push_back(_x & 0xFFFFFFFF);
		_x >>= 32;
	}

	_x = (_x << bitsCount) | rawValue;
}


uint32 NCommon::CANSCoder64::ReadRaw(uint8 bitsCount)
{
	MF_ASSERT(bitsCount > 0);

	uint32 value = (uint32)(_x & NBit::Mask64(bitsCount));
	_x >>= bitsCount;

	if (_x <= 0xFFFFFFFF)
	{
		if (_words.size() > 0)
		{
			_x = (_x << 32) | _words.back();
			_words.pop_back();
		}
	}

	return value;
}


void NCommon::CANSCoder64::Reset()
{
	_x = 0;
	_words.clear();
}


void NCommon::CANSBufferedEncoder::Encode(const CANSDistribution& d, uint32 symbol)
{
	Value value;

	value.distribution = &d;
	value.value = symbol;
	value.bitsCount = 0;

	values.push_back(value);
}


void NCommon::CANSBufferedEncoder::WriteRaw(uint16 rawValue, uint8 bitsCount)
{
	Value value;

	value.distribution = nullptr;
	value.value = rawValue;
	value.bitsCount = bitsCount;

	values.push_back(value);
}


void NCommon::LosslessCompression_UnitTest()
{
	const int n = 100000;
	const bool mixInRawValues = true;
	MF_ASSERT(n % 2 == 0);

	vector<int> tab;
	vector<int> bitsCounts; // 0 - means value is encoded, raw write/read otherwise with given bits count

	for (int i = 0; i < n; i++)
	{
		int bitsCount = 0;
		if (mixInRawValues)
		{
			if (Random16(0, 4) == 0)
				bitsCount = Random16(1, 16);
		}

		bitsCounts.push_back(bitsCount);
	}

	// generate data

	for (int i = 0; i < n; i++)
	{
		int value = 0;

		if (bitsCounts[i] == 0)
		{
			int random = Random16(0, 15);

			if (random >= 10 && random <= 11)
				value = 1;
			else if (random >= 12 && random <= 13)
				value = 2;
			else if (random >= 14 && random <= 15)
				value = 3;
		}
		else
		{
			uint32 mask = (uint32)NBit::Mask64(bitsCounts[i]);
			value = Random16(0, 255) & mask;
		}

		tab.push_back(value);
	}

	// create PDFs

	vector<uint32> pdf1;
	pdf1.push_back(10);
	pdf1.push_back(2);
	pdf1.push_back(2);
	pdf1.push_back(2);

	vector<uint32> pdf2;
	pdf2.push_back(6);
	pdf2.push_back(4);
	pdf2.push_back(4);
	pdf2.push_back(2);

	// Huffman

	int bitsCount1 = 0;
	map<uint32, SDynamicBitArray> huffman1 = Huffman(pdf1);
	map<uint32, SDynamicBitArray> huffman2 = Huffman(pdf2);

	for (int i = 0; i < n; i++)
	{
		if (bitsCounts[i] == 0)
		{
			if (i < n / 2)
				bitsCount1 += huffman1[tab[i]].bitsCount;
			else
				bitsCount1 += huffman2[tab[i]].bitsCount;
		}
		else
		{
			bitsCount1 += bitsCounts[i];
		}
	}

	// ANS

	CANSDistribution d1, d2;

	// ANS32

	vector<int> tab32;
	CANSCoder32 ans32;
	d1.SetPDF(pdf1, 0xFFF + 1);
	d2.SetPDF(pdf2, 0xFFF + 1);

	for (int i = 0; i < n; i++)
	{
		CANSDistribution& d = (i < n/2 ? d1 : d2);

		if (bitsCounts[i] == 0)
			ans32.Encode(d, tab[i]);
		else
			ans32.WriteRaw(tab[i], bitsCounts[i]);
	}

	for (int i = 0; i < n; i++)
	{
		CANSDistribution& d = (i < n/2 ? d2 : d1);
		int bitsCount = bitsCounts[n - 1 - i];

		if (bitsCount == 0)
			tab32.push_back(ans32.Decode(d));
		else
			tab32.push_back(ans32.ReadRaw(bitsCount));
	}

	reverse(tab32.begin(), tab32.end());

	for (int i = 0; i < n; i++)
	{
		MF_ASSERT(tab[i] == tab32[i]);
	}

	// ANS64

	vector<int> tab64;
	CANSCoder64 ans64;
	d1.SetPDF(pdf1, 0xFFFFFF + 1);
	d2.SetPDF(pdf2, 0xFFFFFF + 1);

	for (int i = 0; i < n; i++)
	{
		CANSDistribution& d = (i < n/2 ? d1 : d2);

		if (bitsCounts[i] == 0)
			ans64.Encode(d, tab[i]);
		else
			ans64.WriteRaw(tab[i], bitsCounts[i]);
	}

	for (int i = 0; i < n; i++)
	{
		CANSDistribution& d = (i < n/2 ? d2 : d1);
		int bitsCount = bitsCounts[n - 1 - i];

		if (bitsCount == 0)
			tab64.push_back(ans64.Decode(d));
		else
			tab64.push_back(ans64.ReadRaw(bitsCount));
	}

	reverse(tab64.begin(), tab64.end());

	for (int i = 0; i < n; i++)
	{
		MF_ASSERT(tab[i] == tab64[i]);
	}
}
