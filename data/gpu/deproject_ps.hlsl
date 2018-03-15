#include "samplers.hlsl"


Texture2D<float4> backBufferDepthStencilTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float2 projParams;
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


float4 main(PS_INPUT input): SV_Target
{
	float depth_ndc = backBufferDepthStencilTexture.SampleLevel(pointClampSampler, input.texCoord, 0).x;
	return DepthNDCToView(depth_ndc);
}
