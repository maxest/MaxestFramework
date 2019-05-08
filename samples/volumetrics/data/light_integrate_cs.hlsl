RWTexture3D<float> outputLightIntegratedTexture: register(u0);

Texture3D<float> inputLightVolumeTexture: register(t0);


[numthreads(10, 10, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 10*gID.x + gtID.x;
	uint pixelY = 10*gID.y + gtID.y;

	uint pixelZ = 0;
	float lightIntegrated = 0.0f;
	for (int i = 0; i < LIGHT_VOLUME_TEXTURE_DEPTH; i++)
	{
		uint3 pixelCoord = uint3(pixelX, pixelY, pixelZ);

		lightIntegrated += inputLightVolumeTexture[pixelCoord];
		outputLightIntegratedTexture[pixelCoord] = lightIntegrated;

		pixelZ++;
	}
}
