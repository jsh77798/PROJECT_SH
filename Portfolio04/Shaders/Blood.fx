#include "00. Global.fx"

float BloodTime = 0.f;

struct BloodInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float2 scale : SCALE;
    float3 velocity : VELOCITY;
    float lifetime : LIFETIME;
};

struct BloodOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float alpha : TEXCOORD1;
};

BloodOutput VS(BloodInput input)
{
    BloodOutput output;

    float life = max(input.lifetime, 0.001f);
    float age = saturate(BloodTime / life);
    float time = min(BloodTime, life);

    // 초기 속도 + 중력
    float3 center =
        input.position +
        input.velocity * time +
        float3(0.f, -3.f, 0.f) * time * time;

    float3 cameraRight = normalize(
        mul(float4(1.f, 0.f, 0.f, 0.f), VInv).xyz
    );

    float3 cameraUp = normalize(
        mul(float4(0.f, 1.f, 0.f, 0.f), VInv).xyz
    );

    // 수명이 끝나면서 조금 작아짐
    float sizeScale = lerp(1.f, 0.6f, age);

    float3 position = center;

    position += cameraRight *
        ((input.uv.x - 0.5f) * input.scale.x * sizeScale);

    position += cameraUp *
        ((0.5f - input.uv.y) * input.scale.y * sizeScale);

    output.position = mul(float4(position, 1.f), VP);
    output.uv = input.uv;

    // 수명 후반에 사라짐
    output.alpha = 1.f - smoothstep(0.45f, 1.f, age);

    return output;
}

float4 PS(BloodOutput input) : SV_TARGET
{
    float2 p = input.uv * 2.f - 1.f;

    // 윗부분이 좁은 물방울 형태
    float width = lerp(
        0.35f,
        1.f,
        saturate(input.uv.y * 1.5f)
    );

    float2 shapePosition =
        float2(p.x / width, p.y);

    float radiusSquared =
        dot(shapePosition, shapePosition);

    float shapeAlpha =
        1.f - smoothstep(0.65f, 1.f, radiusSquared);

    float alpha = shapeAlpha * input.alpha;

    clip(alpha - 0.01f);

    float shade = saturate(1.f - radiusSquared);

    float3 color = lerp(
        float3(0.16f, 0.005f, 0.005f),
        float3(0.55f, 0.015f, 0.01f),
        shade
    );

    return float4(color, alpha);
}

DepthStencilState BloodDepth
{
    DepthEnable = true;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
    StencilEnable = false;
};

RasterizerState BloodRasterizer
{
    FillMode = Solid;
    CullMode = None;
    ScissorEnable = false;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(BloodRasterizer);
        SetDepthStencilState(BloodDepth, 0);

        SetBlendState(
            AlphaBlend,
            float4(0.f, 0.f, 0.f, 0.f),
            0xFFFFFFFF
        );

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
};