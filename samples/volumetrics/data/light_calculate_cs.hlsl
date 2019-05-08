RWTexture3D<float> outputLightVolumeTexture: register(u0);


[numthreads(10, 10, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 10*gID.x + gtID.x;
	uint pixelY = 10*gID.y + gtID.y;
	uint pixelZ = 1*gID.z + gtID.z;

	uint3 pixelCoord = uint3(pixelX, pixelY, pixelZ);
	outputLightVolumeTexture[pixelCoord] = 4.0f/64.0f;
}
