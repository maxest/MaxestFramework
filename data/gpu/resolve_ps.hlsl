#ifdef MSAA
	Texture2DMS<float4> inputTexture: register(t0);
#else
	Texture2D<float4> inputTexture: register(t0);
#endif


cbuffer ConstantBuffer: register(b0)
{
	int ssScale;
	int msaaCount;
}


struct PS_INPUT
{
	float4 position: SV_POSITION;
};


float4 main(PS_INPUT input): SV_Target
{
	float4 result = 0.0f;

	for (int y = 0; y < ssScale; y++)
	{
		for (int x = 0; x < ssScale; x++)
		{
			int2 pixelCoord = ssScale*(int2)input.position.xy + int2(x, y);
			
		#ifdef MSAA
			for (int k = 0; k < msaaCount; k++)				
				result += inputTexture.Load(pixelCoord, k);
		#else
			result += inputTexture.Load(int3(pixelCoord, 0));
		#endif
		}
	}

	return result / (ssScale * ssScale * msaaCount);
}
