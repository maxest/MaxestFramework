#include "../../../data/gpu/math.hlsl"
#include "../../../data/gpu/samples.hlsl"
#include "../../../data/gpu/samplers.hlsl"
#include "../../../data/gpu/noise.hlsl"


cbuffer ConstantBuffer: register(b1)
{
	float2 screenSize;
	float2 screenPixelSize;
	float2 checkerPixelSize;
	float shadowFilterMaxSize;
	float penumbraFilterMaxSize;
}


float AvgBlockersDepthToPenumbra(float z_shadowMapView, float avgBlockersDepth)
{
	float penumbra = (z_shadowMapView - avgBlockersDepth) / avgBlockersDepth;
	penumbra *= penumbra;
	return saturate(80.0f * penumbra);
}


float Penumbra(float gradientNoise, Texture2D<float4> shadowMapTexture, float2 shadowMapUV, float z_shadowMapView, int samplesCount)
{
	float avgBlockersDepth = 0.0f;
	float blockersCount = 0.0f;

	for (int i = 0; i < samplesCount; i++)
	{
		float2 sampleUV = VogelDiskOffset(i, samplesCount, gradientNoise);
		sampleUV = shadowMapUV + penumbraFilterMaxSize*sampleUV;

		float sampleDepth = shadowMapTexture.SampleLevel(pointClampSampler, sampleUV, 0).x;

		if (sampleDepth < z_shadowMapView)
		{
			avgBlockersDepth += sampleDepth;
			blockersCount += 1.0f;
		}
	}

	if (blockersCount > 0.0f)
	{
		avgBlockersDepth /= blockersCount;
		return AvgBlockersDepthToPenumbra(z_shadowMapView, avgBlockersDepth);
	}
	else
	{
		return 0.0f;
	}
}
