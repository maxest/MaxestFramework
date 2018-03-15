#include "coding.hlsl"
#include "vertex_structs.hlsl"


cbuffer ConstantBuffer: register(b0)
{
	matrix worldViewProjTransform;
	uint color;
}


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
};


VS_OUTPUT main_vs(Vertex_PosNorUV0UV1 input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.position = mul(worldViewProjTransform, input.position);

	return output;
}


float4 main_ps(VS_OUTPUT input): SV_Target
{
	return Decode_R8G8B8A8UInt_From_R32UInt(color)/255.0f;
}
