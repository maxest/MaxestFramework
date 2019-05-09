RWTexture3D<float> outputLightIntegratedTexture: register(u0);

Texture3D<float> inputLightVolumeTexture: register(t0);
Texture3D<float> inputPrevLightIntegratedTexture: register(t1);


[numthreads(8, 8, 1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
	float lightIntegrated = 0.0f;

	for (int i = 0; i < LIGHT_VOLUME_TEXTURE_DEPTH; i++)
	{
		uint3 pixelCoord = uint3(dtID.xy, i);
		
		lightIntegrated += inputLightVolumeTexture[pixelCoord];
		
		float currLightIntegrated = lightIntegrated;
		float prevLightIntegrated = inputPrevLightIntegratedTexture[pixelCoord];
		currLightIntegrated = lerp(currLightIntegrated, prevLightIntegrated, 0.9f);
		
		outputLightIntegratedTexture[pixelCoord] = currLightIntegrated;
	}
}
