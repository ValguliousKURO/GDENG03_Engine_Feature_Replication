#include "DX3D/Assets/Shaders/Common.hlsl"

cbuffer MaterialData : register(b2)
{
    float3 color;
};

Texture2D Diffuse : register(t0);

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 worldPos = mul(float4(input.position, 1), world);
    output.position = mul(worldPos, view);
    output.position = mul(output.position, proj);
    output.texcoord = input.texcoord;
    output.normal = mul(float4(input.normal, 0), world).xyz;
    output.worldPos = worldPos.xyz;
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET
{
    float4 diffuseTex = Diffuse.Sample(DefaultSampler, input.texcoord);
    
    float3 N = normalize(input.normal);
    float3 L = normalize(-lightDirection.xyz);
    float3 V = normalize(cameraPosition.xyz - input.worldPos);
    float3 H = normalize(L + V);
    
    float3 ambient = ambientColor.rgb;
    
    float diffuseStrength = max(dot(N, L), 0.0f);
    float3 diffuse = diffuseStrength * lightColor.rgb;
    
    float specularStrength = pow(max(dot(N, H), 0.0f), 32.0f);
    float3 specular = specularStrength * lightColor.rgb * 0.5f;
    
    float3 finalColor = color.rgb * diffuseTex.rgb * (ambient + diffuse) + specular;
    return float4(finalColor, 1.0f);
}

