#include "DX3D/Assets/Shaders/Common.hlsl"

cbuffer MaterialData : register(b2)
{
    float3 color;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1), world);
    output.position = mul(output.position, view);
    output.position = mul(output.position, proj);
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    return float4(color.rgb, 1);
}

