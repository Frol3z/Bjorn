// Global uniform buffer
struct GlobalUniformBuffer {
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0)]]
ConstantBuffer<GlobalUniformBuffer> ubo;

// Per-object data
struct ObjectData {
    float4x4 model;
};

[[vk::binding(1)]]
StructuredBuffer<ObjectData> objectBuffer;

// Push constant used to access the objectBuffer
struct PushConsts
{
    uint objectIndex;
};
[[vk::push_constant]] PushConsts pushConsts;

struct VertexInput {
	[[vk::location(0)]] float3 position : POSITION;
	[[vk::location(1)]] float3 color : COLOR;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 color : COLOR;
};

[shader("vertex")]
VertexOutput vertMain(uint vertexId: SV_VertexID, VertexInput input) {
    VertexOutput output;
    float4x4 model = objectBuffer[pushConsts.objectIndex].model;
    output.position = mul(ubo.proj, mul(ubo.view, mul(model, float4(input.position, 1.0))));
    output.color = input.color;
    return output;
}

[shader("pixel")]
float4 fragMain(VertexOutput inVert) : SV_Target {
    return float4(inVert.color, 1.0);
}