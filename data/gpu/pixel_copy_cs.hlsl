RWTexture2D<float4> outputTexture: register(u0);

Texture2D<float4> inputTexture: register(t0);


cbuffer ConstantBuffer: register(b0)
{
	uint width;
	uint height;
	uint2 padding;
}


[numthreads(16, 16, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 16*gID.x + gtID.x;
	uint pixelY = 16*gID.y + gtID.y;
	uint2 pixelCoord = int2(pixelX, pixelY);

	if (pixelCoord.x < width && pixelCoord.y < height)
		outputTexture[pixelCoord] = inputTexture.Load(float3(pixelCoord, 0));
}
