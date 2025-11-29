// Material data
struct MaterialData
{
    float3 albedo;
    float4 specular;
};

[[vk::binding(0, 2)]]
StructuredBuffer<MaterialData> materialBuffer;

struct VertexOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
    uint materialIndex : TEXCOORD0;
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
    MaterialData m = materialBuffer[inVert.materialIndex];
	output.albedo = float4(m.albedo, 1.0);
    output.specular = m.specular;
    output.normal = float4(normalize(inVert.normal), 1.0);
	return output;
}