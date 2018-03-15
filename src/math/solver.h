// https://www.geometrictools.com/Documentation/LeastSquaresFitting.pdf
// http://parallel.bas.bg/ESGI113/final_reports/problem4_fr.pdf
// http://www.math.umn.edu/~olver/num_/lnqr.pdf
// https://mathsupport.mas.ncl.ac.uk/images/9/9a/Numrcl_detrmntn_eignvl_eignvc.pdf


#pragma once


#include "types.h"
#include "vector.h"
#include "matrix.h"
#include "../essentials/assert.h"


namespace NMaxestFramework { namespace NMath
{
	SVector2 SolveLineCoeffs(const SVector2& p1, const SVector2& p2);
	bool SolveQuadraticRoots(float a, float b, float c, float& t1, float &t2);

	SVector2 SolveLeastSquaresLineDirection(const SVector2& linePoint, const SVector2* points, uint pointsCount, uint iterationsCount);
	SVector3 SolveLeastSquaresLineDirection(const SVector3& linePoint, const SVector3* points, uint pointsCount, uint iterationsCount);

	// approximatex quadratic function using gradient descent
	void SolveQuadraticFromPoints(float& a, float& b, float &c, float& error, const SVector2* points, uint pointsCount, uint batchesCount, uint batchSize, uint iterationsCount = 10000, float learnRate = 0.0001f);

	void Solver_SolveQuadraticFromPoints_UnitTest();

	//

	inline SVector2 SolveLineCoeffs(const SVector2& p1, const SVector2& p2)
	{
		float a = (p1.y - p2.y) / (p1.x - p2.x);
		float b = p1.y - a*p1.x;

		return VectorCustom(a, b);
	}

	inline bool SolveQuadraticRoots(float a, float b, float c, float& x1, float &x2)
	{
		float delta = b*b - 4.0f*a*c;

		if (delta < 0.0f)
			return false;

		float deltaSqrt = Sqrt(delta);

		float q;
		if (b < 0.0f)
			q = -0.5f * (b - deltaSqrt);
		else
			q = -0.5f * (b + deltaSqrt);

		x1 = q / a;
		x2 = c / q;

		if (x1 > x2)
			Swap(x1, x2);

		return true;
	}

	inline SVector2 SolveLeastSquaresLineDirection(const SVector2& linePoint, const SVector2* points, uint pointsCount, uint iterationsCount)
	{
		SMatrix matrix;
		SetZeros(matrix);
		for (uint i = 0; i < pointsCount; i++)
		{
			SVector2 diff = points[i] - linePoint;

			matrix.m[0][0] += diff.x * diff.x;
			matrix.m[0][1] += diff.x * diff.y;
			matrix.m[1][0] += diff.y * diff.x;
			matrix.m[1][1] += diff.y * diff.y;
		}

		SVector2 v1 = VectorCustom(matrix.m[0][0], matrix.m[0][1]);
		SVector2 v2 = VectorCustom(matrix.m[1][0], matrix.m[1][1]);
		SVector2 eigenVector = v1 + v2;

		if (LengthSquared(eigenVector) < cEpsilon4)
			return Normalize(cVector2One);

		for (uint i = 0; i < iterationsCount; i++)
		{
			eigenVector = eigenVector * matrix;
			float maxComponent = MaxComponent(Abs(eigenVector));
			MF_ASSERT(maxComponent != 0.0f);
			eigenVector = eigenVector / maxComponent;
		}

		return Normalize(eigenVector);
	}

	inline SVector3 SolveLeastSquaresLineDirection(const SVector3& linePoint, const SVector3* points, uint pointsCount, uint iterationsCount)
	{
		SMatrix matrix;
		SetZeros(matrix);
		for (uint i = 0; i < pointsCount; i++)
		{
			SVector3 diff = points[i] - linePoint;

			matrix.m[0][0] += diff.x * diff.x;
			matrix.m[0][1] += diff.x * diff.y;
			matrix.m[0][2] += diff.x * diff.z;
			matrix.m[1][0] += diff.y * diff.x;
			matrix.m[1][1] += diff.y * diff.y;
			matrix.m[1][2] += diff.y * diff.z;
			matrix.m[2][0] += diff.z * diff.x;
			matrix.m[2][1] += diff.z * diff.y;
			matrix.m[2][2] += diff.z * diff.z;
		}

		SVector3 v1 = VectorCustom(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]);
		SVector3 v2 = VectorCustom(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]);
		SVector3 v3 = VectorCustom(matrix.m[2][0], matrix.m[2][1], matrix.m[2][2]);
		SVector3 eigenVector = v1 + v2 + v3;

		if (LengthSquared(eigenVector) < cEpsilon4)
			return Normalize(cVector3One);

		for (uint i = 0; i < iterationsCount; i++)
		{
			eigenVector = eigenVector * matrix;
			float maxComponent = MaxComponent(Abs(eigenVector));
			MF_ASSERT(maxComponent != 0.0f);
			eigenVector = eigenVector / maxComponent;
		}

		return Normalize(eigenVector);
	}

	// uses stochastic gradient descent
	inline void SolveQuadraticFromPoints(float& a, float& b, float &c, float& error, const SVector2* points, uint pointsCount, uint batchesCount, uint batchSize, uint iterationsCount, float learnRate)
	{
		struct Utils
		{
			float Cost(float x, float y, float a, float b, float c)
			{
				float value = a*x*x + b*x + c - y;
				return value * value;
			}
		} utils;

		MF_ASSERT(pointsCount == batchesCount * batchSize);

		const float derivativeEpsilon = cEpsilon4;

		a = b = c = error = 0.0f;
		for (uint i = 0; i < iterationsCount; i++)
		{
			error = 0.0;
			for (uint j = 0; j < pointsCount; j++)
			{
				float cost = utils.Cost(points[j].x, points[j].y, a, b, c);
				error += cost;
			}
			error /= (float)pointsCount;

			for (uint k = 0; k < batchesCount; k++)
			{
				float dA = 0.0f;
				float dB = 0.0f;
				float dC = 0.0f;
				for (uint j = 0; j < batchSize; j++)
				{
					int pointIndex = k*batchSize + j;

					dA += (utils.Cost(points[pointIndex].x, points[pointIndex].y, a + derivativeEpsilon, b, c) - utils.Cost(points[pointIndex].x, points[pointIndex].y, a - derivativeEpsilon, b, c)) / (2.0f * derivativeEpsilon);
					dB += (utils.Cost(points[pointIndex].x, points[pointIndex].y, a, b + derivativeEpsilon, c) - utils.Cost(points[pointIndex].x, points[pointIndex].y, a, b - derivativeEpsilon, c)) / (2.0f * derivativeEpsilon);
					dC += (utils.Cost(points[pointIndex].x, points[pointIndex].y, a, b, c + derivativeEpsilon) - utils.Cost(points[pointIndex].x, points[pointIndex].y, a, b, c - derivativeEpsilon)) / (2.0f * derivativeEpsilon);
				}
				dA /= (float)batchSize;
				dB /= (float)batchSize;
				dC /= (float)batchSize;

				a -= learnRate * dA;
				b -= learnRate * dB;
				c -= learnRate * dC;
			}
		}
	}

	inline void Solver_SolveQuadraticFromPoints_UnitTest()
	{
		float a = 2.0f;
		float b = -2.0f;
		float c = 4.0f;
		vector<SVector2> points;
		for (int i = 0; i < 100; i++)
		{
			SVector2 p;

			p.x = (float)i / 10.0f;
			p.y = a*p.x*p.x + b*p.x + c;

			p.x += (float)RandomFloat(-0.1f, 0.1f);
			p.y += (float)RandomFloat(-0.1f, 0.1f);

			points.push_back(p);
		}

		a, b, c = 0.0f;
		float error = 0.0f;
		SolveQuadraticFromPoints(a, b, c, error, &points[0], (uint)points.size(), 10, 10);

		cout << error << endl;
		cout << a << "  " << b << "  " << c << endl;
	}
} }
