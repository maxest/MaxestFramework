cbuffer ConstantBuffer: register(b0)
{
	matrix viewProjTransform;
}


struct APP_OUTPUT
{
	float4 position: POSITION;
};


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
};


VS_OUTPUT main(APP_OUTPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	output.position = mul(viewProjTransform, input.position);

	return output;
}
