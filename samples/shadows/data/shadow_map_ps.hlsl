struct PS_INPUT
{
	float4 position: SV_POSITION;
	float z_view: TEXCOORD0;
};


struct PS_OUTPUT
{
	float4 color: SV_Target;
};


PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;

	output.color = -input.z_view;
	
	return output;
}
