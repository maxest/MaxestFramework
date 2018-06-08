uint Encode_R8SInt_In_R8UInt(int r)
{
	return r + 128;
}


int Decode_R8SInt_From_R8UInt(uint value_encoded)
{
	return value_encoded - 128;
}


uint Encode_R8G8SInt_In_R16UInt(int r, int g)
{
	uint r2 = r + 128;
	uint g2 = g + 128;

	return (r2 & 255) | ((g2 & 255) << 8);
}


void Decode_R8G8SInt_From_R16UInt(uint value_encoded, out int r, out int g)
{
	r = (value_encoded & 255) - 128;
	g = ((value_encoded >> 8) & 255) - 128;
}


uint Encode_R8G8B8A8UInt_In_R32UInt(uint r, uint g, uint b, uint a)
{
	uint value_encoded = r & 255;
	value_encoded |= (g & 255) << 8;
	value_encoded |= (b & 255) << 16;
	value_encoded |= (a & 255) << 24;

	return value_encoded;
}


uint4 Decode_R8G8B8A8UInt_From_R32UInt(uint value_encoded)
{
	uint4 value;

	value.r = (value_encoded >> 0) & 255;
	value.g = (value_encoded >> 8) & 255;
	value.b = (value_encoded >> 16) & 255;
	value.a = (value_encoded >> 24) & 255;
	
	return value;
}


uint Encode_2x_R8G8SInt_In_R32UInt(int2 v1, int2 v2)
{
	uint v1_encoded = Encode_R8G8SInt_In_R16UInt(v1.x, v1.y);
	uint v2_encoded = Encode_R8G8SInt_In_R16UInt(v2.x, v2.y);
	
	return (v1_encoded) | (v2_encoded << 16);
}


void Decode_2x_R8G8SInt_From_R32UInt(uint value_encoded, out int2 v1, out int2 v2)
{
	Decode_R8G8SInt_From_R16UInt(value_encoded, v1.x, v1.y);
	Decode_R8G8SInt_From_R16UInt(value_encoded >> 16, v2.x, v2.y);
}


uint4 Encode_RGB9SInt_In_RGBA8UInt(int3 value)
{
	uint4 value_encoded = 0;
	
	value_encoded.xyz = abs(value.xyz);

	if (value.x < 0)
		value_encoded.w += 1;
	if (value.y < 0)
		value_encoded.w += 2;
	if (value.z < 0)
		value_encoded.w += 4;
	
	return value_encoded;
}


int3 Decode_RGB9SInt_From_RGBA8UInt(uint4 value_encoded)
{
	int3 value = value_encoded.xyz;

	if ((value_encoded.w >> 0) & 1)
		value.x *= -1;
	if ((value_encoded.w >> 1) & 1)
		value.y *= -1;
	if ((value_encoded.w >> 2) & 1)
		value.z *= -1;
	
	return value;
}
int3 Decode_RGB9SInt_From_R32UInt(uint value_encoded)
{
	int3 value;

	value.x = (value_encoded >> 0) & 255;
	value.y = (value_encoded >> 8) & 255;
	value.z = (value_encoded >> 16) & 255;

	if ((value_encoded >> 24) & 1)
		value.x *= -1;
	if ((value_encoded >> 25) & 1)
		value.y *= -1;
	if ((value_encoded >> 26) & 1)
		value.z *= -1;		

	return value;
}
