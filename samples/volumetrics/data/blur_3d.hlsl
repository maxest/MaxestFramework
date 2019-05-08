RWTexture3D<float> outputTexture3D: register(u0);

Texture3D<float> inputTexture3D: register(t0);


[numthreads(10, 10, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 10*gID.x + gtID.x;
	uint pixelY = 10*gID.y + gtID.y;
	uint pixelZ = 1*gID.z + gtID.z;
	uint3 pixelCoord = uint3(pixelX, pixelY, pixelZ);
	
	float result = 0.0f;
	[unroll]
	for (int i = -3; i <= 3; i++)
	{
		uint x = pixelX;
		uint y = pixelY;
		uint z = pixelZ;
		
	#if defined(X)
		x += i;
	#elif defined(Y)
		y += i;
	#elif defined(Z)
		z += i;
	#endif
		
		result += inputTexture3D[uint3(x, y, z)];
	}
	
	outputTexture3D[pixelCoord] = result / 7.0f;
}
