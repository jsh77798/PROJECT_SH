#include "00. Global.fx"
#include "00. Light.fx"

#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500

struct KeyframeDesc
{
    int animIndex;
    uint currFrame;
    uint nextFrame;
    float ratio;
    float sumTime;
    float speed;
    float2 padding;
};

struct TweenFrameDesc
{
    float tweenDuration;
    float tweenRatio;
    float tweenSumTime;
    float padding;
    KeyframeDesc curr;
    KeyframeDesc next;
};

cbuffer TweenBuffer
{
    TweenFrameDesc TweenFrames;
    //TweenFrameDesc TweenFrames[MAX_MODEL_INSTANCE];
};

cbuffer BoneBuffer
{
    matrix BoneTransforms[MAX_MODEL_TRANSFORMS];
};

uint BoneIndex;
Texture2DArray TransformMap;

struct VS_IN
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 blendIndices : BLEND_INDICES;
    float4 blendWeights : BLEND_WEIGHTS;
	// INSTANCING;
    uint instanceID : SV_INSTANCEID;
    matrix world : INST;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

matrix GetAnimationMatrix(VS_IN input)
{
    float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
    float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };

    int animIndex[2];
    int currFrame[2];
    int nextFrame[2];
    float ratio[2];

    animIndex[0] = TweenFrames.curr.animIndex;
    currFrame[0] = TweenFrames.curr.currFrame;
    nextFrame[0] = TweenFrames.curr.nextFrame;
    ratio[0] = TweenFrames.curr.ratio;

    animIndex[1] = TweenFrames.next.animIndex;
    currFrame[1] = TweenFrames.next.currFrame;
    nextFrame[1] = TweenFrames.next.nextFrame;
    ratio[1] = TweenFrames.next.ratio;

    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;
    matrix curr = 0;
    matrix next = 0;
    matrix transform = 0;

    
    for (int i = 0; i < 4; i++)
    {
        c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[0], animIndex[0], 0));
        c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[0], animIndex[0], 0));
        c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[0], animIndex[0], 0));
        c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[0], animIndex[0], 0));
        curr = matrix(c0, c1, c2, c3);

        n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], animIndex[0], 0));
        n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], animIndex[0], 0));
        n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], animIndex[0], 0));
        n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], animIndex[0], 0));
        next = matrix(n0, n1, n2, n3);

        matrix result = lerp(curr, next, ratio[0]);
        

		// 다음 애니메이션
        if (animIndex[1] >= 0)
        {
            c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[1], animIndex[1], 0));
            c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[1], animIndex[1], 0));
            c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[1], animIndex[1], 0));
            c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[1], animIndex[1], 0));
            curr = matrix(c0, c1, c2, c3);

            n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], animIndex[1], 0));
            n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], animIndex[1], 0));
            n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], animIndex[1], 0));
            n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], animIndex[1], 0));
            next = matrix(n0, n1, n2, n3);

            matrix nextResult = lerp(curr, next, ratio[1]);
            result = lerp(result, nextResult, TweenFrames.tweenRatio);
        }

        transform += mul(weights[i], result);
    }
    
    return transform;
}

VS_OUT VS(VS_IN input)
{
    //VS_OUT output;
    //
	////output.position = mul(input.position, BoneTransforms[BoneIndex]); // Model Global
    //
    //matrix m = GetAnimationMatrix(input);
    //
    //output.position = mul(input.position, m);
    //output.position = mul(output.position, input.world); // W
    //output.worldPosition = output.position;
    //output.position = mul(output.position, VP);
    //output.uv = input.uv;
    //output.normal = input.normal;
    //
    //return output;
    
    VS_OUT output;

    matrix animationMatrix = GetAnimationMatrix(input);

    // 메시 로컬 → 모델 공간
    float4 modelPosition = mul(
        float4(input.position.xyz, 1.0f),
        BoneTransforms[BoneIndex]
    );

    // 모델 공간에서 애니메이션 적용
    float4 animatedPosition = mul(
        modelPosition,
        animationMatrix
    );

    // 모델 공간 → 월드 공간
    float4 worldPosition = mul(
        animatedPosition,
        input.world
    );

    output.position = mul(worldPosition, VP);
    output.worldPosition = worldPosition.xyz;
    output.uv = input.uv;

    // 현재 PS는 조명을 사용하지 않으므로 기존 처리 유지
    output.normal = input.normal;

    return output;
}

float4 PS(VS_OUT input) : SV_TARGET
{
	//float4 color = ComputeLight(input.normal, input.uv, input.worldPosition);

    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);

    return color;
}

technique11 T0
{
	PASS_VP(P0, VS, PS)
};
