#include "common.hlsl"


RWTexture3D<float4> outputLightVolumeIntegratedTexture: register(u0);

Texture3D<float4> inputLightVolumeTexture: register(t0);


[numthreads(8, 8, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
	float tauAccum = 0.0f;
	float transmittanceAccum = 0.0f;
	float3 inScatteringAccum = 0.0f;

	for (int i = 0; i < LIGHT_VOLUME_TEXTURE_DEPTH; i++)
	{
		uint3 pixelCoord = uint3(dtID.xy, i);

		float stepSize;
		{
			float positionCurrZ_lightVolume = ((float)i + 0.0f) / LIGHT_VOLUME_TEXTURE_DEPTH;
			float positionNextZ_lightVolume = ((float)i + 1.0f) / LIGHT_VOLUME_TEXTURE_DEPTH;
			stepSize = RemapZ(positionNextZ_lightVolume) - RemapZ(positionCurrZ_lightVolume);
		}

		float4 lightVolumeSample = inputLightVolumeTexture[pixelCoord];
		
		float density = lightVolumeSample.w * stepSize;
		float3 inScattering = lightVolumeSample.w * stepSize * lightVolumeSample.xyz;

		tauAccum += density;
		transmittanceAccum = exp(-tauAccum);
		inScatteringAccum += transmittanceAccum * inScattering;

		outputLightVolumeIntegratedTexture[pixelCoord] = float4(inScatteringAccum, transmittanceAccum);
	}
}
