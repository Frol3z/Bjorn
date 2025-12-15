#define MAX_TEXTURES 10 // must match the one in Renderer.hpp
#define MAX_SAMPLERS 2

// Material data
struct MaterialData
{
    float3 albedo;
    float3 specular;
    float4 materialInfo;
};

[[vk::binding(0, 2)]]
StructuredBuffer<MaterialData> materialBuffer;

[[vk::binding(0, 3)]]
Texture2D textures[MAX_TEXTURES];

[[vk::binding(1, 3)]]
SamplerState samplers[MAX_SAMPLERS];

struct VertexOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    uint materialIndex : TEXCOORD1;
};

struct FragmentOutput 
{
	float4 albedo : SV_TARGET0;
	float4 specular : SV_TARGET1;
    float4 materialInfo : SV_TARGET2;
	float4 normal : SV_TARGET3;
};

FragmentOutput main(VertexOutput inVert)
{
	FragmentOutput output;
    MaterialData m = materialBuffer[inVert.materialIndex];
    output.albedo = textures[0].Sample(samplers[0], inVert.uv);
    //output.albedo = float4(m.albedo, 1.0);
    output.specular = float4(m.specular, 1.0);
    output.materialInfo = m.materialInfo;
    output.normal = float4(normalize(inVert.normal), 1.0);
	return output;
}