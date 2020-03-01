#include "common.hlsl"

#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> depthBufferTexture: register(t0);
Texture3D<float4> lightVolumeIntegratedTexture: register(t1);
Texture2D<float4> gbufferDiffuseTexture: register(t2);


cbuffer ConstantBuffer: register(b0)
{
	float2 projParams;
	float nearPlaneDistance;
	float viewDistance;
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


float4 main(PS_INPUT input): SV_Target0
{
	float depthSample_ndc = depthBufferTexture.SampleLevel(pointClampSampler, input.texCoord, 0).x;
	float depth = DepthNDCToView(depthSample_ndc);

	float4 inScattering;
	float transmittance;
	{
		float3 position_lightVolume;
		position_lightVolume.xy = input.texCoord;
		position_lightVolume.z = ViewSpaceToLightVolumeSpaceZ(depth, nearPlaneDistance, viewDistance);
		float4 lightVolumeSample = lightVolumeIntegratedTexture.SampleLevel(linearClampSampler, position_lightVolume, 0);
		
		inScattering = float4(lightVolumeSample.xyz, 0.0f);
		transmittance = lightVolumeSample.w;
	}
	
	float4 gbufferDiffuseSample = gbufferDiffuseTexture.SampleLevel(pointClampSampler, input.texCoord, 0);
//	gbufferDiffuseSample = 1.0f;
	
	return transmittance * gbufferDiffuseSample + inScattering;
}
