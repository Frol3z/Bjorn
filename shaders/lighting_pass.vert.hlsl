struct VertexOutput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

VertexOutput main(uint vertexId : SV_VertexID)
{
    VertexOutput output;
    
    // Draw a full screen triangle
    output.uv = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(output.uv * 2.0 + -1.0, 0.0, 1.0);
    
    return output;
}