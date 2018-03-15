#ifndef CONVERSION_HLSL
#define CONVERSION_HLSL


float2 PixelCoordToTextureCoord(int2 pixelCoord, int width, int height)
{
	return ((float2)pixelCoord + float2(0.5f, 0.5f)) / float2(width, height);
}


int2 TextureCoordToPixelCoord(float2 textureCoord, int width, int height)
{
	return int2(textureCoord * float2(width, height));
}


float3 RGBToLumaChroma(float3 value)
{
	float3x3 transform;
	transform[0][0] = 0.299f;
	transform[0][1] = 0.587f;
	transform[0][2] = 0.114f;
	transform[1][0] = -0.168736f;
	transform[1][1] = -0.331264f;
	transform[1][2] = 0.5f;
	transform[2][0] = 0.5f;
	transform[2][1] = -0.418688f;
	transform[2][2] = -0.081312f;
	
	value = mul(transform, value);
	value.yz += 0.5f;
	
	return value;
}


float3 LumaChromaToRGB(float3 value)
{
	float3x3 transform;
	transform[0][0] = 1.0f;
	transform[0][1] = 0.0f;
	transform[0][2] = 1.402f;
	transform[1][0] = 1.0f;
	transform[1][1] = -0.344136f;
	transform[1][2] = -0.714136f;
	transform[2][0] = 1.0f;
	transform[2][1] = 1.772f;
	transform[2][2] = 0.0f;

	value.yz -= 0.5f;
	value = mul(transform, value);

	return value;
}


#endif
