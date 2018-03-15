#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> currFrameTexture: register(t0);
Texture2D<float4> prevFrameTexture: register(t1);
Texture2D<float4> depthTexture: register(t2);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 reprojectTransform;
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
	float4 position_screen = 1.0f;
	position_screen.xy = 2.0f*input.texCoord - 1.0f;
	position_screen.y *= -1.0f;
	position_screen.z = depthTexture.Load(int3(input.position.xy, 0)).x;

	float4 prevPosition_screen = mul(reprojectTransform, position_screen);
	prevPosition_screen = prevPosition_screen / prevPosition_screen.w;
	float2 prevTexCoord	= float2(0.5f, -0.5f)*prevPosition_screen.xy + 0.5f;

	float3 color_center = currFrameTexture.Load(int3(input.position.xy, 0)).xyz;
	float3 prevColor = prevFrameTexture.SampleLevel(linearClampSampler, prevTexCoord, 0).xyz;
	
#ifdef CLAMP
	float3 color_left = currFrameTexture.Load(int3(input.position.xy + int2(-1, 0), 0)).xyz;
	float3 color_right = currFrameTexture.Load(int3(input.position.xy + int2(1, 0), 0)).xyz;
	float3 color_top = currFrameTexture.Load(int3(input.position.xy + int2(0, -1), 0)).xyz;
	float3 color_bottom = currFrameTexture.Load(int3(input.position.xy + int2(0, 1), 0)).xyz;
	float3 minColor = min(color_left, min(color_right, min(color_top, color_bottom)));
	float3 maxColor = max(color_left, max(color_right, max(color_top, color_bottom)));
	prevColor = clamp(prevColor, minColor, maxColor);
#endif
	
	return float4(lerp(color_center, prevColor, 0.5f), 1.0f);
}
