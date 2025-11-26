struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// Camera uniform buffer (set 0)
struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float4x4 invViewProj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraData;

// G-buffer (set 1)
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
Texture2D gAlbedo;
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
SamplerState gAlbedoSampler;

[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
Texture2D gSpecular;
[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
SamplerState gSpecularSampler;

[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
Texture2D gNormal;
[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
SamplerState gNormalSampler;

[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
Texture2D gDepth;
[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
SamplerState gDepthSampler;

// Hardcoded directional light
static const float3 LIGHT_POS = float3(0.0, 1.0, 0.0);
static const float3 LIGHT_COL = float3(1.0, 0.73, 0.23);

float3 reconstructWorldPosition(float depth, float2 uv)
{
    float2 ndc = uv * 2.0 - 1.0;
    float4 clipPos = float4(ndc.x, ndc.y, depth, 1.0);
    float4 worldPosH = mul(cameraData.invViewProj, clipPos);
    return (worldPosH.xyz / worldPosH.w);
}

float4 main(VertexOutput inVert) : SV_TARGET0
{
    float3 albedo = gAlbedo.Sample(gAlbedoSampler, inVert.uv).rgb;
    float3 normal = normalize(gNormal.Sample(gNormalSampler, inVert.uv).rgb);
    float depth = gDepth.Sample(gDepthSampler, inVert.uv).r;
    //float3 position = reconstructWorldPosition(depth, inVert.uv);
    
    float3 lightDir = -LIGHT_POS;
    float3 outColor = albedo * LIGHT_COL * max(dot(normalize(lightDir), normal), 0.0);
    return float4(outColor, 1.0);
}