#include "../../../data/gpu/samplers.hlsl"


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


float Luminance(float3 value)
{
	return dot(value, float3(0.212f, 0.716f, 0.072f));
}


float4 main(PS_INPUT input): SV_Target
{
	float2 filterSize = 2.0f * pixelSize;
	float luminance_left = Luminance(inputTexture.SampleLevel(linearClampSampler, input.texCoord + filterSize*float2(-1.0f, 0.0f), 0).xyz);
	float luminance_right = Luminance(inputTexture.SampleLevel(linearClampSampler, input.texCoord + filterSize*float2(1.0f, 0.0f), 0).xyz);
	float luminance_top = Luminance(inputTexture.SampleLevel(linearClampSampler, input.texCoord + filterSize*float2(0.0f, -1.0f), 0).xyz);
	float luminance_bottom = Luminance(inputTexture.SampleLevel(linearClampSampler, input.texCoord + filterSize*float2(0.0f, 1.0f), 0).xyz);

	float2 tangent = float2( -(luminance_top - luminance_bottom), (luminance_right - luminance_left) );
	float tangentLength = length(tangent);
	tangent /= tangentLength;
	tangent *= pixelSize;

	float4 color_center = inputTexture.SampleLevel(pointClampSampler, input.texCoord, 0);
	float4 color_left = inputTexture.SampleLevel(linearClampSampler, input.texCoord - 0.5f*tangent, 0);// * 0.666f;
	float4 color_left2 = inputTexture.SampleLevel(linearClampSampler, input.texCoord - tangent, 0);// * 0.333f;
	float4 color_right = inputTexture.SampleLevel(linearClampSampler, input.texCoord + 0.5f*tangent, 0);// * 0.666f;
	float4 color_right2 = inputTexture.SampleLevel(linearClampSampler, input.texCoord + tangent, 0);// * 0.333f;

	float4 color_blurred = (color_center + color_left + color_left2 + color_right + color_right2) / 5.0f;
	float blurStrength = saturate(2.0f * tangentLength);
	
	return lerp(color_center, color_blurred, blurStrength);
}
