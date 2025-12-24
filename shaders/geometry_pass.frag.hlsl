#define MAX_TEXTURES 10 // must match the one in Renderer.hpp
#define MAX_SAMPLERS 2

// Material data
struct MaterialData
{
    float3       baseColor;
    float4    materialInfo;
    
    uint      baseColorTex;   // index to address textures[]
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
	float4 baseColor : SV_TARGET0;
    float4 materialInfo : SV_TARGET1;
	float4 normal : SV_TARGET2;
};

FragmentOutput main(VertexOutput inVert)
{
	FragmentOutput output;
    MaterialData m = materialBuffer[inVert.materialIndex];
    
    // Base Color
    if(m.baseColorTex != -1)
        output.baseColor = textures[m.baseColorTex].Sample(samplers[0], inVert.uv);
    else
        output.baseColor = float4(m.baseColor, 1.0);
    
    // Material Info
    output.materialInfo = m.materialInfo;
    
    // Normal
    output.normal = float4(normalize(inVert.normal), 1.0);
	
    return output;
}