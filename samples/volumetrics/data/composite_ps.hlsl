// https://www.shadertoy.com/view/4df3Dn
// https://www.shadertoy.com/view/MllSzX
// https://developer.nvidia.com/gpugems/gpugems2/part-iii-high-quality-rendering/chapter-20-fast-third-order-texture-filtering
// https://www.paulinternet.nl/?page=bicubic
// https://github.com/DannyRuijters/CubicInterpolationCUDA/blob/master/examples/glCubicRayCast/tricubic.shader


#include "common.hlsl"

#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> depthBufferTexture: register(t0);
Texture3D<float4> lightVolumeIntegratedTexture: register(t1);
Texture2D<float4> gbufferDiffuseTexture: register(t2);


cbuffer ConstantBuffer: register(b0)
{
	float2 projParams;
	float nearPlaneDistance;
	float viewDistance;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float DepthNDCToView(float depth_ndc)
{
	return -projParams.y / (depth_ndc + projParams.x);
}


float4 SampleLightVolume_Trilinear(float3 uvw)
{
	return lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, uvw, 0);
}


float4 SampleLightVolume_Bicubic(float3 uvw)
{
	float3 lightVolumeSize = float3(LIGHT_VOLUME_TEXTURE_WIDTH, LIGHT_VOLUME_TEXTURE_HEIGHT, LIGHT_VOLUME_TEXTURE_DEPTH);
	float3 pixelCoord = uvw * lightVolumeSize + 0.5f;
	uint3 pixelCoordIndex = floor(pixelCoord);
	float3 pixelCoordFrac = frac(pixelCoord);
	float3 onePixelCoordFrac = 1.0f - pixelCoordFrac;
	
	float3 w0 = 1.0f/6.0f * onePixelCoordFrac * onePixelCoordFrac * onePixelCoordFrac;
	float3 w1 = 2.0f/3.0f - 0.5f * pixelCoordFrac * pixelCoordFrac * (2.0f - pixelCoordFrac);
	float3 w2 = 2.0f/3.0f - 0.5f * onePixelCoordFrac * onePixelCoordFrac * (2.0f - onePixelCoordFrac);
	float3 w3 = 1.0f/6.0f * pixelCoordFrac * pixelCoordFrac * pixelCoordFrac;
	
	float3 g0 = w0 + w1;
	float3 g1 = w2 + w3;
	float3 h0 = (1.0f / lightVolumeSize) * ((w1 / g0) - 1.5f + pixelCoordIndex);
	float3 h1 = (1.0f / lightVolumeSize) * ((w3 / g1) + 0.5f + pixelCoordIndex);
	
	h0.z = uvw.z;
	
	float4 tex000 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, h0, 0);
	float4 tex100 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h1.x, h0.y, h0.z), 0);
	tex000 = lerp(tex100, tex000, g0.x);
	
	float4 tex010 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h0.x, h1.y, h0.z), 0);
	float4 tex110 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h1.x, h1.y, h0.z), 0);
	tex010 = lerp(tex110, tex010, g0.x);

	return tex000 = lerp(tex010, tex000, g0.y);
}


float4 SampleLightVolume_Tricubic(float3 uvw)
{
	float3 lightVolumeSize = float3(LIGHT_VOLUME_TEXTURE_WIDTH, LIGHT_VOLUME_TEXTURE_HEIGHT, LIGHT_VOLUME_TEXTURE_DEPTH);
	float3 pixelCoord = uvw * lightVolumeSize + 0.5f;
	uint3 pixelCoordIndex = floor(pixelCoord);
	float3 pixelCoordFrac = frac(pixelCoord);
	float3 onePixelCoordFrac = 1.0f - pixelCoordFrac;
	
	float3 w0 = 1.0f/6.0f * onePixelCoordFrac * onePixelCoordFrac * onePixelCoordFrac;
	float3 w1 = 2.0f/3.0f - 0.5f * pixelCoordFrac * pixelCoordFrac * (2.0f - pixelCoordFrac);
	float3 w2 = 2.0f/3.0f - 0.5f * onePixelCoordFrac * onePixelCoordFrac * (2.0f - onePixelCoordFrac);
	float3 w3 = 1.0f/6.0f * pixelCoordFrac * pixelCoordFrac * pixelCoordFrac;
	
	float3 g0 = w0 + w1;
	float3 g1 = w2 + w3;
	float3 h0 = (1.0f / lightVolumeSize) * ((w1 / g0) - 1.5f + pixelCoordIndex);
	float3 h1 = (1.0f / lightVolumeSize) * ((w3 / g1) + 0.5f + pixelCoordIndex);
	
	float4 tex000 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, h0, 0);
	float4 tex100 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h1.x, h0.y, h0.z), 0);
	tex000 = lerp(tex100, tex000, g0.x);
	
	float4 tex010 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h0.x, h1.y, h0.z), 0);
	float4 tex110 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h1.x, h1.y, h0.z), 0);
	tex010 = lerp(tex110, tex010, g0.x);
	tex000 = lerp(tex010, tex000, g0.y);
	
	float4 tex001 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h0.x, h0.y, h1.z), 0);
	float4 tex101 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h1.x, h0.y, h1.z), 0);
	tex001 = lerp(tex101, tex001, g0.x);
	
	float4 tex011 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, float3(h0.x, h1.y, h1.z), 0);
	float4 tex111 = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, h1, 0);
	tex011 = lerp(tex111, tex011, g0.x);
	tex001 = lerp(tex011, tex001, g0.y);
	
	return lerp(tex001, tex000, g0.z);
}


float W0(float x)
{
	return 1.0f/6.0f * (-x*x*x + 3.0f*x*x - 3.0f*x + 1.0f);
}
float W1(float x)
{
	return 1.0f/6.0f * (3.0f*x*x*x - 6.0f*x*x + 4.0f);
}
float W2(float x)
{
	return 1.0f/6.0f * (-3.0f*x*x*x + 3.0f*x*x + 3.0f*x + 1.0f);
}
float W3(float x)
{
	return 1.0f/6.0f * (x*x*x);
}
float4 CubicInterpolate(float4 p[4], float x)
{
	return p[0]*W0(x) + p[1]*W1(x) + p[2]*W2(x) + p[3]*W3(x);
}
float4 BicubicInterpolate(float4 p[4][4], float x, float y)
{
	float4 values[4];
	values[0] = CubicInterpolate(p[0], y);
	values[1] = CubicInterpolate(p[1], y);
	values[2] = CubicInterpolate(p[2], y);
	values[3] = CubicInterpolate(p[3], y);
	return CubicInterpolate(values, x);
}
float4 TricubicInterpolate(float4 p[4][4][4], float x, float y, float z)
{
	float4 values[4];
	values[0] = BicubicInterpolate(p[0], y, z);
	values[1] = BicubicInterpolate(p[1], y, z);
	values[2] = BicubicInterpolate(p[2], y, z);
	values[3] = BicubicInterpolate(p[3], y, z);
	return CubicInterpolate(values, x);
}
float4 SampleLightVolume_Tricubic_Expensive(float3 uvw)
{
	float3 lightVolumeSize = float3(LIGHT_VOLUME_TEXTURE_WIDTH, LIGHT_VOLUME_TEXTURE_HEIGHT, LIGHT_VOLUME_TEXTURE_DEPTH);
	float3 pixelCoord = uvw * lightVolumeSize - 1.5f;

	uint3 pixelCoordBase = (uint3)(pixelCoord);
	float3 pixelCoordFrac = frac(pixelCoord);
	
	float4 samples[4][4][4];	
	for (int k = 0; k < 4; k++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int i = 0; i < 4; i++)
			{
				uint3 samplePixelCoord = uint3(pixelCoordBase.x + i, pixelCoordBase.y + j, pixelCoordBase.z + k);
				float3 sampleUVW = (float3)(samplePixelCoord + 0.5f) / lightVolumeSize;				
				samples[i][j][k] = lightVolumeIntegratedTexture.SampleLevel(pointClampSampler, sampleUVW, 0);
			}
		}
	}

	return TricubicInterpolate(samples, pixelCoordFrac.x, pixelCoordFrac.y, pixelCoordFrac.z);
}


float4 main(PS_INPUT input): SV_Target0
{
	float depthSample_ndc = depthBufferTexture.SampleLevel(pointClampSampler, input.texCoord, 0).x;
	float depth = DepthNDCToView(depthSample_ndc);

	float4 inScattering;
	float transmittance;
	{
		float3 position_lightVolume;
		position_lightVolume.xy = input.texCoord;
		position_lightVolume.z = ViewSpaceToLightVolumeSpaceZ(depth, nearPlaneDistance, viewDistance);
		
		float4 lightVolumeSample;
	//	lightVolumeSample = SampleLightVolume_Trilinear(position_lightVolume);
	//	lightVolumeSample = SampleLightVolume_Bicubic(position_lightVolume);
		lightVolumeSample = SampleLightVolume_Tricubic(position_lightVolume);
	//	lightVolumeSample = SampleLightVolume_Tricubic_Expensive(position_lightVolume);
		
		inScattering = float4(lightVolumeSample.xyz, 0.0f);
		transmittance = lightVolumeSample.w;
	}
	
	float4 gbufferDiffuseSample = gbufferDiffuseTexture.SampleLevel(pointClampSampler, input.texCoord, 0);
//	gbufferDiffuseSample = 1.0f;
	
	return transmittance * gbufferDiffuseSample + inScattering;
}
