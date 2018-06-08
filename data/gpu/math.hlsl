#ifndef MATH_HLSL
#define MATH_HLSL


static const float Pi = 3.141593f;
static const float TwoPi = 2.0f * Pi;
static const float GoldenAngle = 2.4f;


float2 RotatePoint(float2 pt, float angle)
{
	float sine, cosine;
	sincos(angle, sine, cosine);
	
	float2 rotatedPoint;
	rotatedPoint.x = cosine*pt.x + -sine*pt.y;
	rotatedPoint.y = sine*pt.x + cosine*pt.y;
	
	return rotatedPoint;
}


#endif
