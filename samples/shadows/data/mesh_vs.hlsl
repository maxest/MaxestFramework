cbuffer ConstantBuffer: register(b0)
{
	matrix worldTransform;
	matrix viewProjTransform;
}


struct APP_OUTPUT
{
	float4 position: POSITION;
	float3 normal: NORMAL;
	float2 texCoord: TEXCOORD0;
	float2 texCoordLightmap: TEXCOORD1;
};


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
	float2 texCoord: TEXCOORD0;
	float2 texCoordLightmap: TEXCOORD1;
	float3 normal: TEXCOORD2;
};


VS_OUTPUT main(APP_OUTPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	// world-space
	float4 position_world = mul(worldTransform, input.position);
	float3 normal_world = mul((float3x3)worldTransform, input.normal);
	
	output.position = mul(viewProjTransform, position_world);
	output.texCoord = input.texCoord;
	output.texCoordLightmap = input.texCoordLightmap;
	output.normal = normal_world;

	return output;
}
