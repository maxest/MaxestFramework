#include "../../../data/gpu/samplers.hlsl"


Texture2D<float4> depthBufferTexture: register(t0);
Texture3D<float> lightIntegratedTexture: register(t1);
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


float RemapZInvert(float z)
{
	return 5.0f / 22.0f * log(80.0f * z + 1.0f);
}
float RemapZ2Invert(float z)
{
	return 1000.0f * log(1.0f/667.0f * (10000.0f * z + 667.0f)) / 2773.0f;
}


float4 main(PS_INPUT input): SV_Target0
{
	float depthSample_ndc = depthBufferTexture.SampleLevel(pointClampSampler, input.texCoord, 0).x;
	float depth = DepthNDCToView(depthSample_ndc);

	float3 lightIntegrationTexCoord;
	lightIntegrationTexCoord.xy = input.texCoord;
	lightIntegrationTexCoord.z = (-depth - nearPlaneDistance) / viewDistance; // -depth because we're in RH system
	lightIntegrationTexCoord.z = RemapZ2Invert(lightIntegrationTexCoord.z);
	float lightIntegratedSample = lightIntegratedTexture.SampleLevel(linearClampSampler, lightIntegrationTexCoord, 0);

	float4 gbufferDiffuseSample = gbufferDiffuseTexture.SampleLevel(pointClampSampler, input.texCoord, 0);

	return lightIntegratedSample;
}
