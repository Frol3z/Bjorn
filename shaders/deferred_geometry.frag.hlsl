struct VertexOutput
{
	float4 position : SV_Position;
	float3 color : COLOR;
};

struct FragmentOutput 
{
	float4 albedo : SV_TARGET0;
	float4 specular : SV_TARGET1;
	float4 normal : SV_TARGET2;
};

FragmentOutput main(VertexOutput inVert)
{
	FragmentOutput output;
	output.albedo = float4(inVert.color, 1.0);
	output.specular = float4(1.0, 0.0, 0.0, 1.0);
	output.normal = float4(0.0, 1.0, 0.0, 1.0);
	return output;
}