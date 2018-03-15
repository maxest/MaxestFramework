#define USE_GATHER_DEPTH
#define USE_GATHER_SHADOW_MASK


#include "common.hlsl"
#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> backBufferDepthStencilTexture: register(t0);
Texture2D<float4> shadowMaskCheckerTexture: register(t1);


cbuffer ConstantBuffer: register(b0)
{
	float2 projParams;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float DepthNDCToView(float depth_cameraNDC)
{
	return -projParams.y / (depth_cameraNDC + projParams.x);
}


float2 FullResPixelCoordToCheckerResUV(int2 pixelCoord)
{
	pixelCoord.x /= 2;
	return ((float2)pixelCoord + float2(0.5f, 0.5f)) * checkerPixelSize;
}


float4 main(PS_INPUT input): SV_Target
{
	int2 pixelCoord = (int2)input.position.xy;
	bool checkerboard = (pixelCoord.x % 2 == 0 && pixelCoord.y % 2 == 0) || (pixelCoord.x % 2 == 1 && pixelCoord.y % 2 == 1);
	float2 uv = input.texCoord;

	// depth

#ifndef USE_GATHER_DEPTH
	float2 uv_left = input.texCoord + screenPixelSize*float2(-1.0f, 0.0f);
	float2 uv_right = input.texCoord + screenPixelSize*float2(1.0f, 0.0f);
	float2 uv_top = input.texCoord + screenPixelSize*float2(0.0f, -1.0f);
	float2 uv_bottom = input.texCoord + screenPixelSize*float2(0.0f, 1.0f);

	float depthSample = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, uv, 0).x;
	float depthSample_left = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, uv_left, 0).x;
	float depthSample_right = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, uv_right, 0).x;
	float depthSample_top = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, uv_top, 0).x;
	float depthSample_bottom = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, uv_bottom, 0).x;
#else
	float4 depths1 = backBufferDepthStencilTexture.GatherRed(pointClampSampler, uv - screenPixelSize);
	float4 depths2 = backBufferDepthStencilTexture.GatherRed(pointClampSampler, uv);

	float depthSample = depths2.w;
	float depthSample_left = depths1.x;
	float depthSample_right = depths2.z;
	float depthSample_top = depths1.z;
	float depthSample_bottom = depths2.x;
#endif

	// view-space and positive
#ifndef DEPTH_BUFFER_16
	depthSample = -DepthNDCToView(depthSample);
	depthSample_left = -DepthNDCToView(depthSample_left);
	depthSample_right = -DepthNDCToView(depthSample_right);
	depthSample_top = -DepthNDCToView(depthSample_top);
	depthSample_bottom = -DepthNDCToView(depthSample_bottom);
#else
	depthSample = -depthSample;
	depthSample_left = -depthSample_left;
	depthSample_right = -depthSample_right;
	depthSample_top = -depthSample_top;
	depthSample_bottom = -depthSample_bottom;
#endif

	// checker shadow mask

	float2 checkerUV = FullResPixelCoordToCheckerResUV(pixelCoord);
#ifndef USE_GATHER_SHADOW_MASK
	float sample0 = shadowMaskCheckerTexture.SampleLevel(pointClampSampler, checkerUV, 0);
	float sample1 = shadowMaskCheckerTexture.SampleLevel(pointClampSampler, FullResPixelCoordToCheckerResUV(pixelCoord + int2(-1, 0)), 0).x;
	float sample2 = shadowMaskCheckerTexture.SampleLevel(pointClampSampler, FullResPixelCoordToCheckerResUV(pixelCoord + int2(+1, 0)), 0).x;
	float sample3 = shadowMaskCheckerTexture.SampleLevel(pointClampSampler, FullResPixelCoordToCheckerResUV(pixelCoord + int2(0, -1)), 0).x;
	float sample4 = shadowMaskCheckerTexture.SampleLevel(pointClampSampler, FullResPixelCoordToCheckerResUV(pixelCoord + int2(0, +1)), 0).x;
#else
	float4 samples1 = shadowMaskCheckerTexture.GatherRed(pointClampSampler, checkerUV - checkerPixelSize);
	float4 samples2 = shadowMaskCheckerTexture.GatherRed(pointClampSampler, checkerUV);

	float sample0 = samples2.w;
	float sample1, sample2, sample3, sample4;
	[flatten]
	if (pixelCoord.y % 2 == 0)
	{
		sample1 = samples1.y;
		sample2 = samples2.z;
		sample3 = samples1.z;
		sample4 = samples2.x;
	}
	else
	{
		sample1 = samples1.x;
		sample2 = samples1.y;
		sample3 = samples1.z;
		sample4 = samples2.x;
	}	
#endif

	// combine and output

	[flatten]
	if (checkerboard)
	{
		return sample0;
	}
	else
	{
		float factor = 1.0f;
		float weight_left = factor / (0.001f + abs(depthSample - depthSample_left));
		float weight_right = factor / (0.001f + abs(depthSample - depthSample_right));
		float weight_top = factor / (0.001f + abs(depthSample - depthSample_top));
		float weight_bottom = factor / (0.001f + abs(depthSample - depthSample_bottom));
		float weightsSum = weight_left + weight_right + weight_top + weight_bottom;

		[flatten]
		if (weightsSum < 1.0f)
			return 0.0f;
		else
			return (weight_left*sample1 + weight_right*sample2 + weight_top*sample3 + weight_bottom*sample4) / weightsSum;
	}
}
