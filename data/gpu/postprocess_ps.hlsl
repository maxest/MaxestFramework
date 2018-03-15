// http://dev.theomader.com/gaussian-kernel-calculator/


#include "samplers.hlsl"


#if CHANNELS_COUNT == 1
	#define CHANNELS x
	#define TYPE float
#elif CHANNELS_COUNT == 2
	#define CHANNELS xy
	#define TYPE float2
#elif CHANNELS_COUNT == 3
	#define CHANNELS xyz
	#define TYPE float3
#elif CHANNELS_COUNT == 4
	#define CHANNELS xyzw
	#define TYPE float4
#endif


Texture2D<float4> mainTexture: register(t0);


static float gaussWeightsSigma1[7] =
{
	0.00598f,
	0.060626f,
	0.241843f,
	0.383103f,
	0.241843f,
	0.060626f,
	0.00598f
};

static float gaussWeightsSigma3[7] =
{
	0.106595f,
	0.140367f,
	0.165569f,
	0.174938f,
	0.165569f,
	0.140367f,
	0.106595f
};


cbuffer ConstantBuffer: register(b0)
{
#if defined(POW)
	float value;
	float3 padding;
#elif defined(SCALE_AND_OFFSET)
	float2 scale;
	float2 offset;
#else
	float2 pixelSize;
	int from, to;
#endif
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float4 main(PS_INPUT input): SV_Target
{
	float2 uv = input.texCoord;
#ifdef SCALE_AND_OFFSET
	uv *= scale;
	uv += offset;
#endif

#ifdef COPY_POINT
	return mainTexture.SampleLevel(pointClampSampler, uv, 0);
#endif

#ifdef COPY_LINEAR
	return mainTexture.SampleLevel(linearClampSampler, uv, 0);
#endif

#ifdef POW
	float4 color = mainTexture.SampleLevel(pointClampSampler, uv, 0);
	return pow(abs(color), value);
#endif

#ifdef CHANNELS_COUNT
	TYPE result = 0.0f;
#endif

	float2 direction = 0.0f;
#ifdef HORIZONTAL
	direction = float2(1.0f, 0.0f);
#endif
#ifdef VERTICAL
	direction = float2(0.0f, 1.0f);
#endif

#ifdef MIN
	result = mainTexture.SampleLevel(pointClampSampler, uv, 0).CHANNELS;
	for (int i = from; i <= to; i++)
		result = min(result, mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS);
#endif
#ifdef MIN13
	result = mainTexture.SampleLevel(pointClampSampler, uv, 0).CHANNELS;
	for (int i = -6; i <= 6; i++)
		result = min(result, mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS);
#endif

#ifdef MAX
	result = mainTexture.SampleLevel(pointClampSampler, uv, 0).CHANNELS;
	for (int i = from; i <= to; i++)
		result = max(result, mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS);
#endif
#ifdef MAX13
	result = mainTexture.SampleLevel(pointClampSampler, uv, 0).CHANNELS;
	for (int i = -6; i <= 6; i++)
		result = max(result, mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS);
#endif

#ifdef BLUR
	for (int i = from; i <= to; i++)
		result += mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS;
	result /= (to - from + 1.0f);
#endif
#ifdef BLUR13
	for (int i = -6; i <= 6; i++)
		result += mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS;
	result /= 13.0f;
#endif
#ifdef BLUR_GAUSS
	for (int i = -3; i <= 3; i++)
		result += gaussWeightsSigma3[3 + i] * mainTexture.SampleLevel(pointClampSampler, uv + i*direction*pixelSize, 0).CHANNELS;
#endif

#if CHANNELS_COUNT == 1
	return result.xxxx;
#elif CHANNELS_COUNT == 2
	return float4(result.xy, 0.0f, 0.0f);
#elif CHANNELS_COUNT == 3
	return float4(result.xyz, 0.0f);
#elif CHANNELS_COUNT == 4
	return result.xyzw;
#endif
}
