struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

float4 main(VertexOutput inVert) : SV_Target {
    return float4(inVert.color, 1.0);
}