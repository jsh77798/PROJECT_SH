#include "00. Global.fx"
#include "00. Light.fx"
#include "00. Render.fx"

cbuffer TransformBuffer
{
};

struct VS_IN
{
    float3 Position : POSITION;
};

struct VS_OUT
{
    float4 Position : SV_POSITION;
};

VS_OUT VS(VS_IN input)
{
    VS_OUT output;

    float4 pos = float4(input.Position, 1.f);

    // Local ¡æ World
    pos = mul(pos, W);

    // World ¡æ ViewProjection
    pos = mul(pos, VP);

    output.Position = pos;

    return output;
}

float4 PS(VS_OUT input) : SV_TARGET
{
    return float4(1.f, 0.f, 0.f, 1.f);
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}