#include "00. Global.fx"
#include "00. Light.fx"
#include "00. Render.fx"

cbuffer SnowBuffer
{
    float4 Color;

    float3 Velocity;
    float DrawDistance;

    float3 Origin;
    float Turbulence;

    float3 Extent;
    float Time;
};

struct VertexInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
    float2 scale : SCALE;
    float2 random : RANDOM;
};

struct VertexOutputSnow
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float alpha : TEXCOORD1;
};

VertexOutputSnow VS(VertexInput input)
{
    VertexOutputSnow output;

    float3 area = max(
        Extent,
        float3(0.001f, 0.001f, 0.001f)
    );

    // 월드 단위/초로 이동
    float3 particle =
        input.position + Velocity * Time;

    particle.x +=
        sin(Time + input.random.x * 6.283185f) * Turbulence;

    particle.z +=
        cos(Time + input.random.y * 6.283185f) * Turbulence;

    // 카메라 주변 영역에서 반복
    float3 center = Origin +
        (frac((particle - Origin) / area + 0.5f) - 0.5f)
        * area;

    // 카메라를 향하는 사각형
    float3 cameraRight = normalize(
        mul(float4(1, 0, 0, 0), VInv).xyz
    );

    float3 cameraUp = normalize(
        mul(float4(0, 1, 0, 0), VInv).xyz
    );

    float3 position = center;

    position += cameraRight *
        ((input.uv.x - 0.5f) * input.scale.x);

    position += cameraUp *
        ((0.5f - input.uv.y) * input.scale.y);

    // 이미 월드 좌표이므로 W는 곱하지 않음
    output.position = mul(float4(position, 1.0f), VP);
    output.uv = input.uv;

    float distanceToCamera =
        length(center - CameraPosition());

    // 영역 경계에서 재배치되는 모습 완화
    float3 edge = saturate(
        (area * 0.5f - abs(center - Origin))
        / max(area * 0.1f, float3(0.001f, 0.001f, 0.001f))
    );

    float edgeFade = min(edge.x, min(edge.y, edge.z));

    output.alpha = edgeFade * saturate(
        1.0f - distanceToCamera / max(DrawDistance, 0.001f)
    );

    return output;
}

float4 PS(VertexOutputSnow input) : SV_TARGET
{
    float4 textureColor =
        DiffuseMap.Sample(LinearSampler, input.uv);

    float4 color;
    color.rgb = textureColor.rgb * Color.rgb;
    color.a = textureColor.a * Color.a * input.alpha;

    clip(color.a - 0.01f);

    return color;
}

DepthStencilState SnowDepth
{
    DepthEnable = true;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
};

RasterizerState SnowRasterizer
{
    FillMode = Solid;
    CullMode = None;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(SnowRasterizer);
        SetDepthStencilState(SnowDepth, 0);

        SetBlendState(
            AlphaBlend,
            float4(0, 0, 0, 0),
            0xFFFFFFFF
        );

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
};