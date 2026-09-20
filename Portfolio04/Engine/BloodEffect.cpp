#include "pch.h"
#include "BloodEffect.h"

#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Camera.h"
#include "Scene.h"
#include "MathUtils.h"

void BloodEffect::Init(
    shared_ptr<Shader> shader,
    const Vec3& position,
    const Vec3& direction)
{
    _shader = shader;
    _elapsedTime = 0.f;

    GetOrAddTransform();

    Vec3 sprayDirection = direction;

    if (sprayDirection.LengthSquared() < 0.000001f)
        sprayDirection = Vec3(0.f, 0.f, 1.f);

    sprayDirection.Normalize();

    constexpr uint32 particleCount = 12;

    vector<VertexBlood> vertices;
    vector<uint32> indices;

    vertices.reserve(particleCount * 4);
    indices.reserve(particleCount * 6);

    const Vec2 uvs[4] =
    {
        Vec2(0.f, 1.f),
        Vec2(0.f, 0.f),
        Vec2(1.f, 1.f),
        Vec2(1.f, 0.f)
    };

    for (uint32 i = 0; i < particleCount; ++i)
    {
        // 주 방향에 무작위 퍼짐 추가
        Vec3 velocity =
            sprayDirection * MathUtils::Random(0.7f, 1.8f);

        velocity += Vec3(
            MathUtils::Random(-0.8f, 0.8f),
            MathUtils::Random(0.3f, 1.4f),
            MathUtils::Random(-0.8f, 0.8f)
        );

        const float width =
            MathUtils::Random(0.06f, 0.14f);

        const Vec2 scale(
            width,
            width * MathUtils::Random(1.0f, 1.8f)
        );

        const float lifetime =
            MathUtils::Random(0.25f, _duration);

        const Vec3 startPosition = position + Vec3(
            MathUtils::Random(-0.03f, 0.03f),
            MathUtils::Random(-0.03f, 0.03f),
            MathUtils::Random(-0.03f, 0.03f)
        );

        const uint32 base =
            static_cast<uint32>(vertices.size());

        for (uint32 corner = 0; corner < 4; ++corner)
        {
            VertexBlood vertex = {};

            vertex.position = startPosition;
            vertex.uv = uvs[corner];
            vertex.scale = scale;
            vertex.velocity = velocity;
            vertex.lifetime = lifetime;

            vertices.push_back(vertex);
        }

        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);

        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
    }

    _vertexBuffer = make_shared<VertexBuffer>();
    _vertexBuffer->Create(vertices);

    _indexBuffer = make_shared<IndexBuffer>();
    _indexBuffer->Create(indices);
}

void BloodEffect::Update()
{
    GameObject::Update();

    _elapsedTime += TIME->GetDeltaTime();

    if (_elapsedTime >= _duration)
    {
        CUR_SCENE->Remove(shared_from_this());
    }
}

void BloodEffect::Render()
{
    if (!_shader || !_vertexBuffer || !_indexBuffer)
        return;

    if (_elapsedTime >= _duration)
        return;

    _shader->PushGlobalData(
        Camera::S_MatView,
        Camera::S_MatProjection
    );

    _shader->GetScalar("BloodTime")
        ->SetFloat(_elapsedTime);

    // 렌더 상태 보관
    ComPtr<ID3D11BlendState> oldBlend;
    FLOAT oldBlendFactor[4] = {};
    UINT oldSampleMask = 0;

    DC->OMGetBlendState(
        oldBlend.GetAddressOf(),
        oldBlendFactor,
        &oldSampleMask
    );

    ComPtr<ID3D11DepthStencilState> oldDepth;
    UINT oldStencilRef = 0;

    DC->OMGetDepthStencilState(
        oldDepth.GetAddressOf(),
        &oldStencilRef
    );

    ComPtr<ID3D11RasterizerState> oldRasterizer;
    DC->RSGetState(oldRasterizer.GetAddressOf());

    ComPtr<ID3D11GeometryShader> oldGeometry;
    DC->GSGetShader(
        oldGeometry.GetAddressOf(),
        nullptr,
        nullptr
    );

    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    DC->IAGetPrimitiveTopology(&oldTopology);

    _vertexBuffer->PushData();
    _indexBuffer->PushData();

    DC->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    );

    _shader->DrawIndexed(
        0,
        0,
        _indexBuffer->GetCount()
    );

    // 상태 복원
    DC->OMSetBlendState(
        oldBlend.Get(),
        oldBlendFactor,
        oldSampleMask
    );

    DC->OMSetDepthStencilState(
        oldDepth.Get(),
        oldStencilRef
    );

    DC->RSSetState(oldRasterizer.Get());

    DC->GSSetShader(
        oldGeometry.Get(),
        nullptr,
        0
    );

    DC->IASetPrimitiveTopology(oldTopology);
}