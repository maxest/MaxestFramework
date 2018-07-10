struct APP_OUTPUT
{
	uint index: SV_VertexID;
};


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
};


VS_OUTPUT main(APP_OUTPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	if (input.index == 0)
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 1.0f);
	}
	else if (input.index == 1)
	{
		output.position = float4(1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 1.0f);
	}
	else if (input.index == 2)
	{
		output.position = float4(1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 0.0f);
	}
	else if (input.index == 3)
	{
		output.position = float4(-1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 0.0f);
	}

	return output;
}
