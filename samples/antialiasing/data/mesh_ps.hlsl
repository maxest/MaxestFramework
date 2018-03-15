#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> diffuseTexture: register(t0);
Texture2D<float4> lightmapColorTexture: register(t1);
Texture2D<float4> lightmapDirTexture: register(t2);


struct PS_INPUT
{
	float4 position: SV_POSITION;
	float3 normal: TEXCOORD0;
	float2 texCoord: TEXCOORD1;
#ifdef LIGHTMAPPED
	float2 texCoordLightmap: TEXCOORD2;
#endif
};


struct PS_OUTPUT
{
	float4 color: SV_Target;
};


float3 Unity_DecodeLightmap(float4 color)
{
	return (5.0f * pow(color.a, 1.0f)) * color.rgb;
}


float3 Unity_DecodeDirectionalLightmap(float3 color, float4 dir, float3 normalWorld)
{
	float halfLambert = dot(normalWorld, dir.xyz - 0.5f) + 0.5f;
	return color * halfLambert / max(1e-4h, dir.w);
}


float3 Unity_Lightmap(PS_INPUT input)
{
#ifdef LIGHTMAPPED
	float4 lightmapColorSample = lightmapColorTexture.SampleLevel(linearClampSampler, input.texCoordLightmap, 0);
	float3 lightmap = Unity_DecodeLightmap(lightmapColorSample);

	float4 lightmapDirSample = lightmapDirTexture.SampleLevel(linearClampSampler, input.texCoordLightmap, 0);
	float3 lightmapDir = Unity_DecodeDirectionalLightmap(lightmap, lightmapDirSample, input.normal);

	return lightmapDir;
#else
	return 1.0f;
#endif
}


PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	
	//
	
	input.normal = normalize(input.normal);
	float4 albedo = diffuseTexture.Sample(anisotropicWrapSampler, input.texCoord);

	//

	float3 lightDir = normalize(float3(-2.0f, -8.0f, -2.0f));
	float NdotL = saturate(dot(-lightDir, input.normal));
	float lighting = NdotL;
	lighting = 2.0f * pow(lighting, 4.0f);
	output.color = lighting * albedo;

	//
	
	output.color.xyz = albedo.xyz * Unity_Lightmap(input);

	//
	
	return output;
}
