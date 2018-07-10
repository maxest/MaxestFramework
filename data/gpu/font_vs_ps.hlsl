#include "samplers.hlsl"
#include "coding.hlsl"


Texture2D<float4> mainTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float2 screenSize;
	uint color;
	uint padding;
}


struct APP_OUTPUT
{
	float4 position: POSITION;
	float2 texCoord: TEXCOORD0;
};


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


VS_OUTPUT main_vs(APP_OUTPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.position = input.position;
	output.position.x = 2.0f*input.position.x/screenSize.x - 1.0f;
	output.position.y = -2.0f*input.position.y/screenSize.y + 1.0f;

	output.texCoord = input.texCoord;

	return output;
}


float4 main_ps(VS_OUTPUT input): SV_Target
{
	return Decode_R8G8B8A8UInt_From_R32UInt(color)/255.0f * mainTexture.Sample(anisotropicWrapSampler, input.texCoord);
}
