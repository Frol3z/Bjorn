struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// G-buffer
[[vk::combinedImageSampler]][[vk::binding(0)]]
Texture2D gAlbedo;
[[vk::combinedImageSampler]][[vk::binding(0)]]
SamplerState gAlbedoSampler;

[[vk::combinedImageSampler]][[vk::binding(1)]]
Texture2D gSpecular;
[[vk::combinedImageSampler]][[vk::binding(1)]]
SamplerState gSpecularSampler;

[[vk::combinedImageSampler]][[vk::binding(2)]]
Texture2D gNormal;
[[vk::combinedImageSampler]][[vk::binding(2)]]
SamplerState gNormalSampler;

[[vk::combinedImageSampler]][[vk::binding(3)]]
Texture2D gDepth;
[[vk::combinedImageSampler]][[vk::binding(3)]]
SamplerState gDepthSampler;

float4 main(VertexOutput inVert) : SV_TARGET0
{
    float4 outColor = float4(inVert.uv, 1.0, 1.0);
    outColor = gAlbedo.Sample(gAlbedoSampler, inVert.uv);
	return outColor;
}