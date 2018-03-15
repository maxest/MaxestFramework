cbuffer ConstantBuffer: register(b0)
{
	matrix worldViewTransform;
	matrix worldViewProjTransform;
}


struct APP_OUTPUT
{
	float4 position: POSITION;
	float3 normal: NORMAL;
	float2 texCoord: TEXCOORD0;
};


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float z_view: TEXCOORD0;
};


VS_OUTPUT main(APP_OUTPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.position = mul(worldViewProjTransform, input.position);
	output.z_view = mul(worldViewTransform, input.position).z;

	return output;
}
