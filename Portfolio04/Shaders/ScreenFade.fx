float FadeAlpha = 0.f;

struct FadeOutput
{
    float4 position : SV_POSITION;
};

FadeOutput VS(uint vertexID : SV_VertexID)
{
    FadeOutput output;

    // È­¸é ÀüÃ¼¸¦ µ¤´Â »ï°¢Çü
    float2 positions[3] =
    {
        float2(-1.f, -1.f),
        float2(-1.f, 3.f),
        float2(3.f, -1.f)
    };

    output.position =
        float4(positions[vertexID], 0.f, 1.f);

    return output;
}

float4 PS(FadeOutput input) : SV_TARGET
{
    return float4(0.f, 0.f, 0.f, saturate(FadeAlpha));
}

RasterizerState FadeRasterizer
{
    FillMode = Solid;
    CullMode = None;
    ScissorEnable = false;
    DepthClipEnable = true;
};

DepthStencilState FadeDepth
{
    DepthEnable = false;
    DepthWriteMask = Zero;
    StencilEnable = false;
};

BlendState FadeBlend
{
    AlphaToCoverageEnable = false;

    BlendEnable[0] = true;

    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;

    SrcBlendAlpha = One;
    DestBlendAlpha = Inv_Src_Alpha;
    BlendOpAlpha = Add;

    RenderTargetWriteMask[0] = 15;
};

technique11 T0
{
    pass P0
    {
        SetRasterizerState(FadeRasterizer);
        SetDepthStencilState(FadeDepth, 0);
        SetBlendState(
            FadeBlend,
            float4(0.f, 0.f, 0.f, 0.f),
            0xffffffff
        );

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
};