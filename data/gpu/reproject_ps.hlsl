#include "samplers.hlsl"


Texture2D<float4> mainTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 reprojectTransform;
	float width, height;
	float2 padding;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float2 Reproject(float2 pixelCoord, float2 uv)
{
//	float2 uv = pixelCoord / float2(width, height);
	float depth = 1.0f;
	{
		uv.y = 1.0f - uv.y;
		uv = 2.0f*uv - 1.0f;

		float4 uv2 = float4(uv, depth, 1.0f);
		uv2 = mul(reprojectTransform, uv2);
		uv2 /= uv2.w;

		uv = uv2.xy;
		uv = 0.5f*uv + 0.5f;
		uv.y = 1.0f - uv.y;
	}
	
	return uv;
}


float4 main(PS_INPUT input): SV_Target
{
	float2 uv = Reproject(input.position.xy, input.texCoord);
	return mainTexture.SampleLevel(linearClampSampler, uv, 0);
}
