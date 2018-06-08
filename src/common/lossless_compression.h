#pragma once


#include "../essentials/bit.h"
#include "../essentials/array.h"


namespace NMaxestFramework { namespace NCommon
{
	// Huffman

	map<uint32, NEssentials::SDynamicBitArray> Huffman(const vector<uint32>& pdf);

	// ANS

	class CANSDistribution
	{
	public:
		void SetPDF(const vector<uint32>& pdf, uint64 pdfSum);
		void SetPDF32(const vector<uint32>& pdf);
		void SetPDF64(const vector<uint32>& pdf);

		uint32 Symbol(uint64 x) const;

	public:
		vector<uint32> _pdf;
		vector<uint64> _cdf_exclusive; // exclusive, ie. _cdf(i) doesn't store _pdf(i)
		uint64 _pdfSum;
		uint64 _pdfSumLog2;
	};

	class CANSCoder32
	{
	public:
		CANSCoder32() { _x = 0; }

		void Encode(const CANSDistribution& d, uint32 symbol);
		uint32 Decode(const CANSDistribution& d);

		void WriteRaw(uint16 rawValue, uint8 bitsCount);
		uint16 ReadRaw(uint8 bitsCount);

		void Reset();

	public:
		uint32 _x;
		deque<uint16> _words;
	};

	class CANSCoder64
	{
	public:
		CANSCoder64() { _x = 0; }

		void Encode(const CANSDistribution& d, uint32 symbol);
		uint32 Decode(const CANSDistribution& d);

		void WriteRaw(uint32 rawValue, uint8 bitsCount);
		uint32 ReadRaw(uint8 bitsCount);

		void Reset();

	public:
		uint64 _x;
		deque<uint32> _words;
	};

	class CANSBufferedEncoder
	{
	private:
		struct Value
		{
			const CANSDistribution* distribution;
			uint32 value;
			uint8 bitsCount;
		};

	public:
		void Encode(const CANSDistribution& d, uint32 symbol);
		void WriteRaw(uint16 rawValue, uint8 bitsCount);
		template<typename ANS_CODER> void Flush(ANS_CODER& ansCoder);

	private:
		vector<Value> values;
	};

	//

	template<typename ANS_CODER> void CANSBufferedEncoder::Flush(ANS_CODER& ansCoder)
	{
		reverse(values.begin(), values.end());

		for (uint i = 0; i < values.size(); i++)
		{
			if (values[i].distribution == nullptr)
				ansCoder.WriteRaw(values[i].value, values[i].bitsCount);
			else
				ansCoder.Encode(*values[i].distribution, values[i].value);
		}
	}

	//

	void LosslessCompression_UnitTest();
} }
