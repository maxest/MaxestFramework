#include "../../../data/gpu/samplers.hlsl"

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_GREEN_AS_LUMA 1
#define FXAA_QUALITY__PRESET 12
#include "../../../data/gpu/fxaa_3.11.hlsl"


Texture2D<float4> inputTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float2 pixelSize;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


float4 main(PS_INPUT input): SV_Target
{
	FxaaTex fxaaTexture;
	fxaaTexture.tex = inputTexture;
	fxaaTexture.smpl = linearClampSampler;

	return FxaaPixelShader(
		input.texCoord,
		float4(0.0f, 0.0f, 0.0f, 0.0f),
		fxaaTexture,
		fxaaTexture,
		fxaaTexture,
		pixelSize,
		float4(0.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 0.0f, 0.0f),
		float4(0.0f, 0.0f, 0.0f, 0.0f),
		0.75f,
		0.166f,
		0.0f,
		0.0f,
		0.0f,
		0.0f,
		float4(0.0f, 0.0f, 0.0f, 0.0f)
	);
}
