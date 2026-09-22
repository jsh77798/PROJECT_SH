#include "00. Global.fx"
#include "00. Light.fx"

float4 SkyColor = float4(0.4f, 0.4f, 0.4f, 1.f);

float UseSkyTexture = 0.f;

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_OUT VS(VertexTextureNormalTangent input)
{
    VS_OUT output;

    // w = 0: 뷰 행렬의 이동 성분 무시
    float4 viewPos = mul(
        float4(input.position.xyz, 0.f),
        V
    );

    float4 clipPosition = mul(viewPos, P);

    // 깊이를 가장 먼 곳인 1로 설정
    output.position = clipPosition.xyww;
    output.uv = input.uv;

    return output;
}

float4 PS(VS_OUT input) : SV_TARGET
{
    float3 textureColor =
        DiffuseMap.Sample(LinearSampler, input.uv).rgb;

    float3 color = lerp(
        SkyColor.rgb,
        textureColor,
        saturate(UseSkyTexture)
    );

    return float4(color, 1.f);
}

DepthStencilState SkyDepth
{
    DepthEnable = true;
    DepthWriteMask = ZERO;
    DepthFunc = LESS_EQUAL;
};

RasterizerState SkyRasterizer
{
    FillMode = Solid;

    // 내부에서 보이도록 우선 양면 렌더링
    CullMode = None;
};

BlendState SkyBlend
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 15;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(SkyRasterizer);
        SetDepthStencilState(SkyDepth, 0);

        SetBlendState(
            SkyBlend,
            float4(0, 0, 0, 0),
            0xFFFFFFFF
        );

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
};