// Global uniform buffer
struct GlobalUniformBuffer {
    float4x4 view;
    float4x4 proj;
};

[vk::binding(0)]
ConstantBuffer<GlobalUniformBuffer> ubo;

// Per-object data
struct ObjectData {
    float4x4 model;
};

[vk::binding(1)]
StructuredBuffer<ObjectData> objectBuffer;

// Push constant used to access the objectBuffer
[vk_push_constant]
cbuffer PushConstants
{
    uint objectIndex;
};

struct VertexInput {
    float3 position;
    float3 color;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 color;
};

[shader("vertex")]
VertexOutput vertMain(uint vertexId: SV_VertexID, VertexInput input) {
    VertexOutput output;
    float4x4 model = objectBuffer[objectIndex].model;
    output.position = mul(ubo.proj, mul(ubo.view, mul(model, float4(input.position, 1.0))));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
    return float4(inVert.color, 1.0);
}