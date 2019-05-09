RWTexture3D<float> outputLightVolumeTexture: register(u0);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 viewToWorldTransform;
	float2 nearPlaneSize;
	float nearPlaneDistance;
	float viewDistance;
}


// https://www.wolframalpha.com/input/?i=y+%3D+(a*e%5E(b*x)+-+1)*c,+0+%3D+(a*e%5E(b*0)+-+1)*c,+0.1+%3D+(a*e%5E(b*0.5)+-+1)*c,+1+%3D+(a*e%5E(b)+-+1)*c
// f(0) = 0, f(0.5) = 0.1, f(1) = 1; means half of layers occupy 10% of view distance
float RemapZ(float z)
{
	return 0.0125f * (exp(4.4f * z) - 1.0f);
}
// f(0) = 0, f(0.5) = 0.2, f(1) = 1; means half of layers occupy 20% of view distance
float RemapZ2(float z)
{
	return 0.0667f * (exp(2.773f * z) - 1.0f);
}


[numthreads(10, 10, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 10*gID.x + gtID.x;
	uint pixelY = 10*gID.y + gtID.y;
	uint pixelZ = 1*gID.z + gtID.z;
	uint3 pixelCoord = uint3(pixelX, pixelY, pixelZ);

	float z = ((float)pixelZ + 0.5f) / (float)LIGHT_VOLUME_TEXTURE_DEPTH;
	z = RemapZ2(z);
	float volumeSliceSize = z; // approximate the current volume slice's size; layers closer are smaller while layers farther are bigger
	z = viewDistance*z + nearPlaneDistance;

	float x = ((float)pixelX + 0.5f) / (float)LIGHT_VOLUME_TEXTURE_WIDTH;
	x -= 0.5f;
	x *= nearPlaneSize.x;
	x = x * z / nearPlaneDistance;	

	float y = ((float)pixelY + 0.5f) / (float)LIGHT_VOLUME_TEXTURE_HEIGHT;
	y -= 0.5f;
	y = -y; // invert Y
	y *= nearPlaneSize.y;
	y = y * z / nearPlaneDistance;

	float4 position_view = float4(x, y, -z, 1.0f); // -z because we're in RH system
	float4 position_world = mul(viewToWorldTransform, position_view);

	volumeSliceSize *= 0.25f;
	//volumeSliceSize = 0.025f;
	if (position_world.x > 0.0f)
		outputLightVolumeTexture[pixelCoord] = volumeSliceSize / 4.0f;
	else
		outputLightVolumeTexture[pixelCoord] = volumeSliceSize / 16.0f;
}
