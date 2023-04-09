struct VertexOut
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

VertexOut FullscreenVS(const uint vertexID : SV_VertexID)
{
    VertexOut output;

    const float x = (float)(vertexID / 2);
    const float y = (float)(vertexID % 2);

    output.position.x = x * 4.0f - 1.0f;
    output.position.y = y * 4.0f - 1.0f;
    output.position.z = 0.0f;
    output.position.w = 1.0f;

    output.uv.x = x * 2.0f;
    output.uv.y = 1.0f - y * 2.0f;

    return output;
}