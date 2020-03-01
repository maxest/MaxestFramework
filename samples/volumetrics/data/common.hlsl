float RemapZ(float z)
{
	return z * z;
	
	// https://www.wolframalpha.com/input/?i=y+%3D+(a*e%5E(b*x)+-+1)*c,+0+%3D+(a*e%5E(b*0)+-+1)*c,+0.1+%3D+(a*e%5E(b*0.5)+-+1)*c,+1+%3D+(a*e%5E(b)+-+1)*c
	// f(0) = 0, f(0.5) = 0.1, f(1) = 1; means half of layers occupy 10% of view distance
	//return 0.0125f * (exp(4.4f * z) - 1.0f);
	// f(0) = 0, f(0.5) = 0.2, f(1) = 1; means half of layers occupy 20% of view distance
	//return 0.0667f * (exp(2.773f * z) - 1.0f);
}


float RemapZInvert(float z)
{
	return sqrt(z);

	//return 5.0f / 22.0f * log(80.0f * z + 1.0f);
	//return 1000.0f * log(1.0f/667.0f * (10000.0f * z + 667.0f)) / 2773.0f;
}


float3 LightVolumeSpaceToViewSpace(float x, float y, float z, float2 nearPlaneSize, float nearPlaneDistance, float viewDistance)
{
	z = RemapZ(z);
	z = viewDistance*z + nearPlaneDistance;

	x -= 0.5f;
	x *= nearPlaneSize.x;
	x = x * z / nearPlaneDistance;

	y -= 0.5f;
	y = -y; // invert Y
	y *= nearPlaneSize.y;
	y = y * z / nearPlaneDistance;

	return float3(x, y, -z); // -z because we're in RH system
}
float3 LightVolumeSpaceToViewSpace(float3 position, float2 nearPlaneSize, float nearPlaneDistance, float viewDistance)
{
	return LightVolumeSpaceToViewSpace(position.x, position.y, position.z, nearPlaneSize, nearPlaneDistance, viewDistance);
}


float3 ViewSpaceToLightVolumeSpace(float x, float y, float z, float2 nearPlaneSize, float nearPlaneDistance, float viewDistance)
{	
	z = -z; // -z because we're in RH system

	x = x * nearPlaneDistance / z;
	x /= nearPlaneSize.x;
	x += 0.5f;

	y = y * nearPlaneDistance / z;
	y /= nearPlaneSize.y;
	y = -y; // invert Y
	y += 0.5f;

	z = (z - nearPlaneDistance) / viewDistance;
	z = RemapZInvert(z);

	return float3(x, y, z);
}
float3 ViewSpaceToLightVolumeSpace(float3 position, float2 nearPlaneSize, float nearPlaneDistance, float viewDistance)
{
	return ViewSpaceToLightVolumeSpace(position.x, position.y, position.z, nearPlaneSize, nearPlaneDistance, viewDistance);
}


float3 ViewSpaceToLightVolumeSpaceZ(float z, float nearPlaneDistance, float viewDistance)
{
	z = (-z - nearPlaneDistance) / viewDistance; // -z because we're in RH system
	return RemapZInvert(z);
}
