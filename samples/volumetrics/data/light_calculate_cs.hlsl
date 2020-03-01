#include "common.hlsl"

#include "../../../data/gpu/samplers.hlsl"
#include "../../../data/gpu/math.hlsl"
#include "../../../data/gpu/noise.hlsl"


RWTexture3D<float4> outputLightVolumeTexture: register(u0);

Texture3D<float4> inputPrevLightVolumeTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 viewToWorldTransform;
	float4x4 viewReprojectTransform; // reprojects from current frame's view space to previous frame's view space
	float2 nearPlaneSize;
	float nearPlaneDistance;
	float viewDistance;
	float3 dither;
	float padding1;
	float3 eyePosition;
	float padding2;
}


float SchlickPhase(float cosTheta, float k)
{
	float a = 1.0f - k*k;
	float b = 1.0f - k*cosTheta;
	return a / (4.0f * Pi * b * b);
}


[numthreads(4, 4, 4)]
void main(uint3 dtID : SV_DispatchThreadID)
{
	uint3 pixelCoord = dtID;

	float3 lightVolumeSize = float3(LIGHT_VOLUME_TEXTURE_WIDTH, LIGHT_VOLUME_TEXTURE_HEIGHT, LIGHT_VOLUME_TEXTURE_DEPTH);

	float3 noise = 0.0f;
	noise.z = frac(InterleavedGradientNoise((float2)pixelCoord.xy + 0.5f) + dither.z);
	noise.x = frac(InterleavedGradientNoise((float2)pixelCoord.yz + 0.5f) + dither.x) - 0.5f;
	noise.y = frac(InterleavedGradientNoise((float2)pixelCoord.xz + 0.5f) + dither.y) - 0.5f;
	noise.xy = 0.0f; // remove to have dither in XY (adds some more noise which is a bit visible but helps with some occasional banding)

	float4 position_world;
	{
		float3 position_lightVolume = ((float3)pixelCoord + 0.5f + noise) / lightVolumeSize;
		float4 position_view = float4(LightVolumeSpaceToViewSpace(position_lightVolume, nearPlaneSize, nearPlaneDistance, viewDistance), 1.0f);
		position_world = mul(viewToWorldTransform, position_view);
	}

	float3 positionToCamera = normalize(eyePosition - position_world.xyz);
	float3 lightDir = normalize(float3(-1.0f, -1.0f, 0.0f));
	float cosTheta = dot(positionToCamera, lightDir);
	float schlickPhase = SchlickPhase(cosTheta, 0.5f);

	float3 inScattering =
		float3(0.15f, 0.0f, 0.0f) +
		2.0f * float3(0.5f, 0.5f, 0.5f) * schlickPhase;

	float density = 0.0f;		
	if (position_world.x > 0.0f)
		density = 8.0f;
	else
		density = 0.0f;

	float4 lightVolume = float4(inScattering, density);

	// temporal
	{
		// for temporal we need un-dithered position
		float3 position_lightVolume = ((float3)pixelCoord + 0.5f) / lightVolumeSize;
		float4 position_view = float4(LightVolumeSpaceToViewSpace(position_lightVolume, nearPlaneSize, nearPlaneDistance, viewDistance), 1.0f);

		// reproject and get back to light volume space
		float4 prevPosition_view = mul(viewReprojectTransform, position_view);
		float3 prevPosition_lightVolume = ViewSpaceToLightVolumeSpace(prevPosition_view.xyz, nearPlaneSize, nearPlaneDistance, viewDistance);

		// sample
		float4 prevLightVolumeSample = inputPrevLightVolumeTexture.SampleLevel(linearClampSampler, prevPosition_lightVolume, 0);

		bool prevLightVolumeSampleValid =
			prevPosition_lightVolume.x >= 0.0f && prevPosition_lightVolume.x <= 1.0f &&
			prevPosition_lightVolume.y >= 0.0f && prevPosition_lightVolume.y <= 1.0f;

		lightVolume = lerp(lightVolume, prevLightVolumeSample, prevLightVolumeSampleValid ? 0.9f : 0.0f);
	}

	outputLightVolumeTexture[pixelCoord] = lightVolume;
}
