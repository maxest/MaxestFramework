#include "common.hlsl"


RWTexture3D<float> outputLightIntegratedTexture: register(u0);

Texture3D<float> inputLightVolumeTexture: register(t0);


[numthreads(8, 8, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
	float tau = 0.0f;

	for (int i = 0; i < LIGHT_VOLUME_TEXTURE_DEPTH; i++)
	{
		uint3 pixelCoord = uint3(dtID.xy, i);

		float stepSize;
		{
			float positionCurrZ_lightVolume = ((float)i + 0.0f) / LIGHT_VOLUME_TEXTURE_DEPTH;
			float positionNextZ_lightVolume = ((float)i + 1.0f) / LIGHT_VOLUME_TEXTURE_DEPTH;
			stepSize = RemapZ(positionNextZ_lightVolume) - RemapZ(positionCurrZ_lightVolume);
		}

		float fogDensity = inputLightVolumeTexture[pixelCoord] * stepSize;

		tau += fogDensity;
		outputLightIntegratedTexture[pixelCoord] = tau;
	}
}
