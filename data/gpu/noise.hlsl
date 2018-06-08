#ifndef NOISE_HLSL
#define NOISE_HLSL


float InterleavedGradientNoise(float2 position_screen)
{
	float3 magic = float3(0.06711056f, 0.00583715f, 52.9829189f);
	return frac(magic.z * frac(dot(position_screen, magic.xy)));
}


float InterleavedGradientNoise4(float2 position_screen)
{
	position_screen.y *= 4.0f;
	return InterleavedGradientNoise(position_screen);
}


float AlchemyNoise(int2 position_screen)
{
	return 30.0f*(position_screen.x^position_screen.y) + 10.0f*(position_screen.x*position_screen.y);
}


#endif
