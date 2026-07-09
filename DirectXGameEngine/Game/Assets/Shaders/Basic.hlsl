#include "DX3D/Assets/Shaders/Common.hlsl"

cbuffer MaterialData : register(b2)
{
    float3 color;
};

Texture2D Diffuse : register(t0);

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.position = mul(float4(input.position, 1), world);
    output.position = mul(output.position, view);
    output.position = mul(output.position, proj);
    output.texcoord = input.texcoord;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 diffuse = Diffuse.Sample(DefaultSampler, input.texcoord);
    return float4(color.rgb * diffuse.rgb, 1);
}

