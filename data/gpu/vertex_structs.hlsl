struct Vertex_Pos
{
	float4 position: POSITION;
};


struct Vertex_PosUV
{
	float4 position: POSITION;
	float2 texCoord: TEXCOORD0;
};


struct Vertex_PosNorUV
{
	float4 position: POSITION;
	float3 normal: NORMAL;
	float2 texCoord: TEXCOORD0;
};


struct Vertex_PosNorUV0UV1
{
	float4 position: POSITION;
	float3 normal: NORMAL;
	float2 texCoord0: TEXCOORD0;
	float2 texCoord1: TEXCOORD1;
};
