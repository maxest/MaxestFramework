#define USE_GATHER


#include "samplers.hlsl"


Texture2D<float4> mainTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float2 pixelSize;
	float2 padding;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float4 main(PS_INPUT input): SV_Target
{
#if defined(_2X2)
	#ifdef USE_GATHER
		float2 uv = input.texCoord + float2(-0.25f*pixelSize.x, -0.25f*pixelSize.y);
		float4 samples = mainTexture.GatherRed(pointClampSampler, uv);

		float sample00 = samples.w;
		float sample10 = samples.z;
		float sample01 = samples.x;
		float sample11 = samples.y;
	#else
		float2 uv00 = input.texCoord + float2(-0.25f*pixelSize.x, -0.25f*pixelSize.y);
		float2 uv10 = input.texCoord + float2(+0.25f*pixelSize.x, -0.25f*pixelSize.y);
		float2 uv01 = input.texCoord + float2(-0.25f*pixelSize.x, +0.25f*pixelSize.y);
		float2 uv11 = input.texCoord + float2(+0.25f*pixelSize.x, +0.25f*pixelSize.y);

		float sample00 = mainTexture.SampleLevel(pointClampSampler, uv00, 0).x;
		float sample10 = mainTexture.SampleLevel(pointClampSampler, uv10, 0).x;
		float sample01 = mainTexture.SampleLevel(pointClampSampler, uv01, 0).x;
		float sample11 = mainTexture.SampleLevel(pointClampSampler, uv11, 0).x;
	#endif

	float result = FUNCTION(sample00, sample10);
	result = FUNCTION(result, sample01);
	result = FUNCTION(result, sample11);
	return result;
#elif defined(_4X4)
	#ifdef USE_GATHER
		float2 uv00 = input.texCoord + float2(-0.375f*pixelSize.x, -0.375f*pixelSize.y);
		float2 uv10 = input.texCoord + float2(+0.125f*pixelSize.x, -0.375f*pixelSize.y);
		float2 uv01 = input.texCoord + float2(-0.375f*pixelSize.x, +0.125f*pixelSize.y);
		float2 uv11 = input.texCoord + float2(+0.125f*pixelSize.x, +0.125f*pixelSize.y);

		float4 samples1 = mainTexture.GatherRed(pointClampSampler, uv00);
		float4 samples2 = mainTexture.GatherRed(pointClampSampler, uv10);
		float4 samples3 = mainTexture.GatherRed(pointClampSampler, uv01);
		float4 samples4 = mainTexture.GatherRed(pointClampSampler, uv11);

		float4 results = FUNCTION(samples1, samples2);
		results = FUNCTION(results, samples3);
		results = FUNCTION(results, samples4);
		
		float result = FUNCTION(results.x, results.y);
		result = FUNCTION(result, results.z);
		result = FUNCTION(result, results.w);
		return result;
	#else
		float2 uv = input.texCoord + float2(-0.375f*pixelSize.x, -0.375f*pixelSize.y);
		float result = mainTexture.SampleLevel(pointClampSampler, uv, 0).x;
		[unroll]
		for (int y = 0; y < 4; y++)
		{
			[unroll]
			for (int x = 0; x < 4; x++)
			{
				float sample = mainTexture.SampleLevel(pointClampSampler, uv, 0, int2(x, y)).x;
				result = FUNCTION(result, sample);
			}
		}
		return result;
	#endif
#endif
}
