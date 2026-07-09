struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

sampler DefaultSampler : register(s0);

cbuffer ObjectData : register(b0)
{
    row_major float4x4 world;
};

cbuffer CameraData : register(b1)
{
    row_major float4x4 view;
    row_major float4x4 proj;
};