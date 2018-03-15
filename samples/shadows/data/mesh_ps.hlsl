#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> diffuseTexture: register(t0);
Texture2D<float4> shadowMaskTexture: register(t1);


cbuffer ConstantBuffer: register(b0)
{
	float3 lightDirection;
	float padding1;
	float2 screenSize;
	float2 padding2;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
	float2 texCoordLightmap: TEXCOORD1;
	float3 normal: TEXCOORD2;
};


float4 main(PS_INPUT input): SV_Target
{
	input.normal = normalize(input.normal);
	float2 screenUV = input.position.xy / screenSize;
	float4 albedo = diffuseTexture.Sample(anisotropicWrapSampler, input.texCoord);

	// lighting
	float NdotL = saturate(dot(-lightDirection, input.normal));
	float lighting = NdotL;
	lighting = 1.0f * pow(lighting, 1.0f);
	float4 color = lighting * albedo;

	// shadows
	float shadow = shadowMaskTexture.SampleLevel(linearClampSampler, screenUV, 0).x;
#ifdef SHADOW_ONLY
	return shadow;
#endif
	color *= shadow;

	// ambient
	color.xyz += 0.2f * albedo.xyz;

	//
	
	return color;
}
