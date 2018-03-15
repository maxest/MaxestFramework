#include "vertex_structs.hlsl"


struct VS_OUTPUT
{
	float4 position: SV_POSITION;
};


VS_OUTPUT main(VERTEX_STRUCTURE input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.position = input.position;

	return output;
}
