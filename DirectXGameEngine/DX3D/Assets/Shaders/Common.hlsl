struct VSInput
{
    float3 position : POSITION0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

cbuffer ObjectData : register(b0)
{
    row_major float4x4 world;
};

cbuffer CameraData : register(b1)
{
    row_major float4x4 view;
    row_major float4x4 proj;
};