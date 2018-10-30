// BT color conversion functions were extracted from NVidia decode sample's CUDA conversion program
// see also here: https://trac.ffmpeg.org/wiki/colorspace


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
	value.y += 0.5f;
	value.z += 0.5f;
	
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

	value.y -= 0.5f;
	value.z -= 0.5f;

	return mul(transform, value);
}


float3 RGBToLumaChroma_BT601(float3 value)
{
	float3x3 transform;
	transform[0][0] = 0.256788254f;
	transform[0][1] = 0.504129469f;
	transform[0][2] = 0.0979058892f;
	transform[1][0] = -0.144914359f;
	transform[1][1] = -0.284497440f;
	transform[1][2] = 0.429411799f;
	transform[2][0] = 0.429411799f;
	transform[2][1] = -0.359578788f;
	transform[2][2] = -0.0698330179f;

	value = mul(transform, value);
	value.x += 16.0f / 255.0f;
	value.y += 128.0f / 255.0f;
	value.z += 128.0f / 255.0f;

	return value;
}


float3 LumaChromaToRGB_BT601(float3 value)
{
	float3x3 transform;
	transform[0][0] = 1.16438353f;
	transform[0][1] = 0.0f;
	transform[0][2] = 1.63246572f;
	transform[1][0] = 1.16438353f;
	transform[1][1] = -0.400706619f;
	transform[1][2] = -0.831528485f;
	transform[2][0] = 1.16438353f;
	transform[2][1] = 2.06328773f;
	transform[2][2] = 0.0f;

	value.x -= 16.0f / 255.0f;
	value.y -= 128.0f / 255.0f;
	value.z -= 128.0f / 255.0f;

	return mul(transform, value);
}


float3 RGBToLumaChroma_BT709(float3 value)
{
	float3x3 transform;
	transform[0][0] = 0.182585895f;
	transform[0][1] = 0.614230633f;
	transform[0][2] = 0.0620070547f;
	transform[1][0] = -0.0983972251f;
	transform[1][1] = -0.331014544f;
	transform[1][2] = 0.429411799f;
	transform[2][0] = 0.429411799f;
	transform[2][1] = -0.390037209f;
	transform[2][2] = -0.0393745564f;

	value = mul(transform, value);
	value.x += 16.0f / 255.0f;
	value.y += 128.0f / 255.0f;
	value.z += 128.0f / 255.0f;

	return value;
}


float3 LumaChromaToRGB_BT709(float3 value)
{
	float3x3 transform;
	transform[0][0] = 1.16438353f;
	transform[0][1] = 0.0f;
	transform[0][2] = 1.83367121f;
	transform[1][0] = 1.16438353f;
	transform[1][1] = -0.218117282f;
	transform[1][2] = -0.545076191f;
	transform[2][0] = 1.16438353f;
	transform[2][1] = 2.16063023f;
	transform[2][2] = 0.0f;

	value.x -= 16.0f / 255.0f;
	value.y -= 128.0f / 255.0f;
	value.z -= 128.0f / 255.0f;

	return mul(transform, value);
}


float3 RGBToLumaChroma_BT2020(float3 value)
{
	float3x3 transform;
	transform[0][0] = 0.224735081f;
	transform[0][1] = 0.580016613f;
	transform[0][2] = 0.0507300682f;
	transform[1][0] = -0.119450986f;
	transform[1][1] = -0.308289915f;
	transform[1][2] = 0.427740902f;
	transform[2][0] = 0.427740872f;
	transform[2][1] = -0.393338263f;
	transform[2][2] = -0.0344025977f;

	value = mul(transform, value);
	value.x += 16.0f / 255.0f;
	value.y += 128.0f / 255.0f;
	value.z += 128.0f / 255.0f;

	return value;
}


float3 LumaChromaToRGB_BT2020(float3 value)
{
	float3x3 transform;
	transform[0][0] = 1.16893196f;
	transform[0][1] = 0.0f;
	transform[0][2] = 1.72370708f;
	transform[1][0] = 1.16893196f;
	transform[1][1] = -0.192351416f;
	transform[1][2] = -0.667873025f;
	transform[2][0] = 1.16893196f;
	transform[2][1] = 2.19922853f;
	transform[2][2] = 0.0f;

	value.x -= 16.0f / 255.0f;
	value.y -= 128.0f / 255.0f;
	value.z -= 128.0f / 255.0f;

	return mul(transform, value);
}


#endif
