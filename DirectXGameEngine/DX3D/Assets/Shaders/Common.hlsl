struct VSInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 worldPos : TEXCOORD1;
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
    float4 cameraPosition;
};

cbuffer LightData : register(b3)
{
    float4 lightDirection;
    float4 lightColor;
    float4 ambientColor;
};