#define MAX_TEXTURES 10 // must match the one in Renderer.hpp
#define MAX_SAMPLERS 2

// Material data
struct MaterialData
{
    float3       albedo;
    float3     specular;
    float4 materialInfo;
    
    uint      albedoTex;   // index to address textures[]
    uint    specularTex;   // index to address textures[]
};

[[vk::binding(0, 2)]]
StructuredBuffer<MaterialData> materialBuffer;

[[vk::binding(0, 3)]]
Texture2D textures[MAX_TEXTURES];

[[vk::binding(1, 3)]]
SamplerState samplers[MAX_SAMPLERS]; // NOTE: currently always defaulting to samplers[0]

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
    
    // Albedo
    if(m.albedoTex != -1)
        output.albedo = textures[m.albedoTex].Sample(samplers[0], inVert.uv);
    else
        output.albedo = float4(m.albedo, 1.0);
        
    // Specular
    if (m.specularTex != -1)
        output.specular = textures[m.specularTex].Sample(samplers[0], inVert.uv);
    else
        output.specular = float4(m.specular, 1.0);
    
    // Material Info
    output.materialInfo = m.materialInfo;
    
    // Normal
    output.normal = float4(normalize(inVert.normal), 1.0);
	
    return output;
}