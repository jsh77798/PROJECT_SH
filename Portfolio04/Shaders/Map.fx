#include "00. Global.fx"
#include "00. Light.fx"
#include "00. Render.fx"

// 안개 설정: 거리는 엔진의 월드 단위
float4 FogColor = float4(0.40f, 0.40f, 0.40f, 1.f);
static const float FogStart = 5.0f;
static const float FogEnd = 30.0f;

float4 PS(MeshOutput input) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);

    // 카메라부터 현재 픽셀까지의 거리
    float distanceToCamera =
        length(input.worldPosition - CameraPosition());

    // 가까우면 0, 멀어질수록 1
    float fogRatio = saturate(
        (distanceToCamera - FogStart) /
        max(FogEnd - FogStart, 0.001f)
    );

    // 원래 투명도는 유지하고 색상에만 안개 적용
    color.rgb = lerp(color.rgb, FogColor.rgb, fogRatio);

    return color;
}

technique11 T0
{
    PASS_VP(P0, VS_Mesh, PS)
    PASS_VP(P1, VS_Model, PS)
    PASS_VP(P2, VS_Animation, PS)
};