#include "common.hlsl"

#include "../../../data/gpu/samplers.hlsl"
#include "../../../data/gpu/noise.hlsl"


RWTexture3D<float> outputLightVolumeTexture: register(u0);

Texture3D<float> inputPrevLightVolumeTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 viewToWorldTransform;
	float4x4 viewReprojectTransform; // reprojects from current frame's view space to previous frame's view space
	float2 nearPlaneSize;
	float nearPlaneDistance;
	float viewDistance;
	float3 dither;
	float padding;
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

	float volumeSliceSize;
	float4 position_world;
	{
		float3 position_lightVolume = ((float3)pixelCoord + 0.5f + noise) / lightVolumeSize;
		float4 position_view = float4(LightVolumeSpaceToViewSpace(position_lightVolume, nearPlaneSize, nearPlaneDistance, viewDistance, volumeSliceSize), 1.0f);
		position_world = mul(viewToWorldTransform, position_view);
	}

	volumeSliceSize *= 0.25f;
	volumeSliceSize *= 4.0f;
	//volumeSliceSize = 0.025f;

	float light = 0.0f;		
	if (position_world.x > 0.0f)
		light = volumeSliceSize / 4.0f;
	else
		light = volumeSliceSize / 16.0f;
	//light = 0.25f * saturate(position_world.x * volumeSliceSize / 50.0f);
	//light = 0.025f * saturate(sin(position_world.x / 10.0f));

	// temporal
	{
		// for temporal we need un-dithered position
		float3 position_lightVolume = ((float3)pixelCoord + 0.5f) / lightVolumeSize;
		float4 position_view = float4(LightVolumeSpaceToViewSpace(position_lightVolume, nearPlaneSize, nearPlaneDistance, viewDistance, volumeSliceSize), 1.0f);

		// get to light volume space
		float4 prevPosition_view = mul(viewReprojectTransform, position_view);
		float3 prevPosition_lightVolume = ViewSpaceToLightVolumeSpace(prevPosition_view.xyz, nearPlaneSize, nearPlaneDistance, viewDistance);
		
		//!! invalidate if invalid

		// sample
		float prevLight = inputPrevLightVolumeTexture.SampleLevel(linearClampSampler, prevPosition_lightVolume, 0);
		light = lerp(light, prevLight, 0.9f);
	}

	outputLightVolumeTexture[pixelCoord] = light;
}
