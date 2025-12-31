#define MAX_TEXTURES 10 // must match the one in Renderer.hpp
#define MAX_SAMPLERS 2
#define PI 3.14159265358979323846

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
Texture2D gBaseColor;
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
SamplerState gBaseColorSampler;

[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
Texture2D gMaterialInfo;
[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
SamplerState gMaterialInfoSampler;

[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
Texture2D gNormal;
[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
SamplerState gNormalSampler;

[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
Texture2D gDepth;
[[vk::combinedImageSampler]][[vk::binding(3, 1)]]
SamplerState gDepthSampler;

[[vk::binding(0, 2)]]
SamplerState samplers[MAX_SAMPLERS]; // NOTE: currently always defaulting to samplers[0]

[[vk::binding(1, 2)]]
Texture2D textures[MAX_TEXTURES];

[[vk::binding(2, 2)]]
TextureCube skybox;

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

// Metalness-weighted Lambertian diffuse BRDF
float3 diffuseBRDF(const float3 albedo, const float metalness)
{
    return (1.0 - metalness) * (float3) (1.0 / PI) * albedo;
}

/** 
 *  Schlick's approximation of Fresnel equations
 * 
 *  NOTES:
 *  - hDotV is the cosine of the angle between the 
 *    sampled microfacet normal and the view direction
 *    (clamped between 0 and 1)
 *  - metals should provide the base color as f0, while
 *    dielectrics should use a value of 0.04 as a good
 *    approximation of their behaviour (see Real
 *    Time Rendering)
 *
 * OPTIMIZATIONS:
 * - S. Lagarde, "Spherical Gaussian approximation for
 *   Blinn-Phong, Phong and Fresnel", 2012
**/ 
float3 F(float3 f0, float hDotV)
{
    float3 f90 = float3(1.0, 1.0, 1.0);
    return f0 + (f90 - f0) * pow(1 - hDotV, 5.0);
}

// GGX normal distribution function
float D(float alphaSquared, float nDotH)
{
    float b = ((alphaSquared - 1.0f) * nDotH * nDotH + 1.0f);
    return alphaSquared / (PI * b * b);
}

// Smith G2 term (masking-shadowing function) for GGX distribution
// Height correlated version - optimized by substituing 
// G_Lambda for G_Lambda_GGX and dividing by (4 * NdotL * NdotV) to cancel out 
// the terms in specular BRDF denominator
// Source: "Moving Frostbite to Physically Based Rendering" by Lagarde & de Rousiers
// Note that returned value is G2 / (4 * NdotL * NdotV) and therefore includes division by specular BRDF denominator
// 
// REFERENCE: https://boksajak.github.io/files/CrashCourseBRDF.pdf
float G(float alphaSquared, float nDotL, float nDotV)
{
    float a = nDotV * sqrt(alphaSquared + nDotL * (nDotL - alphaSquared * nDotL));
    float b = nDotL * sqrt(alphaSquared + nDotV * (nDotV - alphaSquared * nDotV));
    return 0.5f / (a + b);
}

float4 main(VertexOutput inVert) : SV_TARGET0
{
    float3 baseColor = gBaseColor.Sample(gBaseColorSampler, inVert.uv).rgb;
    //float3 spec = gSpecular.Sample(gSpecularSampler, inVert.uv).rgb;
    float4 rawMaterialInfo = gMaterialInfo.Sample(gMaterialInfoSampler, inVert.uv);
    float roughness = rawMaterialInfo.g;
    float metalness = rawMaterialInfo.b;
    float ambient = 0.02;
    float3 n = normalize(gNormal.Sample(gNormalSampler, inVert.uv).rgb);
    float depth = gDepth.Sample(gDepthSampler, inVert.uv).r;
    float3 fragWorldPosition = reconstructWorldPosition(depth, inVert.uv);
    float3 l = -normalize(LIGHT_DIR);
    float3 v = normalize(cameraData.position - fragWorldPosition);
    float3 h = normalize(l + v);
    float nDotL = max(dot(l, n), 0.0);
    float nDotH = max(dot(n, h), 0.0);
    float nDotV = max(dot(n, v), 0.0);
    float hDotV = max(dot(h, v), 0.0);
    
    // BRDF evaluation
    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor, metalness);
    float3 fresnel = F(f0, hDotV);
    float3 specularBRDF = fresnel * D(alphaSquared, nDotH) * G(alphaSquared, nDotL, nDotV);
    
    // TODO: add environment mapping
    //float3 r = reflect(-v, n);
    //float rDotN = max(dot(r, v), 0.0);
    //float3 indirectLighting = F(f0, rDotN) * skybox.Sample(samplers[1], r).rgb; // Environment map
    
    float3 combinedBRDF = (float3(1.0, 1.0, 1.0) - fresnel) * diffuseBRDF(baseColor, metalness) + specularBRDF;
    float3 directLighting = LIGHT_COL * combinedBRDF * nDotL;
    
    return float4(directLighting, 1.0);
}