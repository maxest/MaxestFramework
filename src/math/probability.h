#pragma once


namespace NMaxestFramework { namespace NMath
{
	vector<uint64> CDF(const vector<uint32>& pdf, bool inclusive = true);
	vector<uint32> PDF(const vector<uint64>& cdf_inclusive);

	vector<uint64> CDFNormalized(const vector<uint64>& cdf_inclusive, uint64 pdfSum);
	vector<uint32> PDFNormalized(const vector<uint32>& pdf, uint64 pdfSum);

	void PDFForceNonZero(vector<uint32>& pdf, bool keepSumUnchanged = false);

	//

	inline vector<uint64> CDF(const vector<uint32>& pdf, bool inclusive)
	{
		vector<uint64> cdf;
		uint64 pdfSum = 0;

		for (uint32 i = 0; i < (uint32)pdf.size(); i++)
		{
			if (inclusive)
			{
				pdfSum += pdf[i];
				cdf.push_back(pdfSum);
			}
			else
			{
				cdf.push_back(pdfSum);
				pdfSum += pdf[i];
			}
		}

		return cdf;
	}

	inline vector<uint32> PDF(const vector<uint64>& cdf_inclusive)
	{
		vector<uint32> pdf;
		
		pdf.push_back((uint32)cdf_inclusive[0]);
		for (uint32 i = 0; i < (uint32)cdf_inclusive.size() - 1; i++)
		{
			uint32 diff = (uint32)(cdf_inclusive[i + 1] - cdf_inclusive[i]);
			pdf.push_back(diff);
		}

		return pdf;
	}

	inline vector<uint64> CDFNormalized(const vector<uint64>& cdf_inclusive, uint64 pdfSum)
	{
		vector<uint64> newCDF;

		for (uint32 i = 0; i < cdf_inclusive.size(); i++)
		{
			double fraction = (double)cdf_inclusive[i] / (double)cdf_inclusive.back();
			uint64 value = (uint64)(fraction * pdfSum);
			newCDF.push_back(value);
		}

		return newCDF;
	}

	inline vector<uint32> PDFNormalized(const vector<uint32>& pdf, uint64 pdfSum)
	{
		vector<uint64> cdf = CDF(pdf);
		cdf = CDFNormalized(cdf, pdfSum);
		return PDF(cdf);
	}

	inline void PDFForceNonZero(vector<uint32>& pdf, bool keepSumUnchanged)
	{
		if (keepSumUnchanged)
		{
			for (uint i = 0; i < pdf.size(); i++)
			{
				if (pdf[i] == 0)
				{
					pdf[i] = 1;

					for (;;)
					{
						// find random element and subtract 1 to keep sum
						int indexOfElementToDecrement = Random32(0, (int)pdf.size() - 1);

						if (pdf[indexOfElementToDecrement] > 1)
						{
							pdf[indexOfElementToDecrement]--;
							break;
						}
					}
				}
			}
		}
		else
		{
			for (uint i = 0; i < pdf.size(); i++)
			{
				if (pdf[i] == 0)
					pdf[i] = 1;
			}
		}
	}
} }
