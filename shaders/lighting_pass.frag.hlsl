struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// Camera uniform buffer (set 0)
struct CameraData
{
    float3 position;
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
Texture2D gMaterialInfo;
[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
SamplerState gMaterialInfoSampler;

[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
Texture2D gNormal;
[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
SamplerState gNormalSampler;

[[vk::combinedImageSampler]][[vk::binding(4, 1)]]
Texture2D gDepth;
[[vk::combinedImageSampler]][[vk::binding(4, 1)]]
SamplerState gDepthSampler;

// Hardcoded directional light
static const float3 LIGHT_DIR = float3(1.0, 1.0, -1.0);
static const float3 LIGHT_COL = float3(1.0, 1.0, 1.0);

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
    float3 spec = gSpecular.Sample(gSpecularSampler, inVert.uv).rgb;
    float4 rawMaterialInfo = gMaterialInfo.Sample(gMaterialInfoSampler, inVert.uv);
    float kA = rawMaterialInfo.r;
    float kD = rawMaterialInfo.g;
    float kS = rawMaterialInfo.b;    
    float shininess = rawMaterialInfo.a * 255.0;
    float3 n = normalize(gNormal.Sample(gNormalSampler, inVert.uv).rgb);
    float depth = gDepth.Sample(gDepthSampler, inVert.uv).r;
    float3 fragWorldPosition = reconstructWorldPosition(depth, inVert.uv);
    
    // Blinn-Phong
    float3 l = -normalize(LIGHT_DIR);
    float3 v = normalize(cameraData.position - fragWorldPosition);
    float3 h = normalize(l + v);
    float nDotL = max(dot(l, n), 0.0);
    float nDotH = max(dot(n, h), 0);
    
    float3 diffuse = kD * albedo * nDotL;
    float3 specular = kS * spec * pow(nDotH, shininess);
    float3 ambient = kA * albedo;
    return float4(LIGHT_COL * (ambient + diffuse + specular), 1.0);
}