RWTexture3D<float> outputLightVolumeTexture: register(u0);


cbuffer ConstantBuffer: register(b0)
{
	float4x4 viewToWorldTransform;
	float2 nearPlaneSize;
	float nearPlaneDistance;
	float viewDistance;
}


[numthreads(10, 10, 1)]
void main(uint3 gID: SV_GroupID, uint3 gtID: SV_GroupThreadID)
{
	uint pixelX = 10*gID.x + gtID.x;
	uint pixelY = 10*gID.y + gtID.y;
	uint pixelZ = 1*gID.z + gtID.z;

	float z = ((float)pixelZ + 0.5f) / (float)LIGHT_VOLUME_TEXTURE_DEPTH;
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
	position_world /= position_world.w;

	uint3 pixelCoord = uint3(pixelX, pixelY, pixelZ);
	if (position_world.x > 0.0f)
		outputLightVolumeTexture[pixelCoord] = 4.0f/64.0f;
	else
		outputLightVolumeTexture[pixelCoord] = 1.0f/64.0f;
}
