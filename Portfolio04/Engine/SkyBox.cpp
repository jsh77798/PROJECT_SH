#include "pch.h"
#include "Skybox.h"

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Camera.h"

#include <cmath>

void Skybox::Init(
    shared_ptr<Shader> shader,
    shared_ptr<Texture> texture)
{
    assert(shader != nullptr);
    assert(texture != nullptr);

    _shader = shader;

    _material = make_shared<Material>();
    _material->SetShader(_shader);
    _material->SetDiffuseMap(texture);

    constexpr uint32 slices = 64;
    constexpr uint32 stacks = 32;

    vector<SkyVertex> vertices;
    vector<uint32> indices;

    vertices.reserve((slices + 1) * (stacks + 1));
    indices.reserve(slices * stacks * 6);

    // 원점 중심의 반지름 1인 구체
    for (uint32 y = 0; y <= stacks; ++y)
    {
        float v = static_cast<float>(y) / stacks;
        float phi = v * XM_PI;

        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (uint32 x = 0; x <= slices; ++x)
        {
            float u = static_cast<float>(x) / slices;
            float theta = u * XM_2PI;

            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            Vec3 position(
                sinPhi * cosTheta,
                cosPhi,
                sinPhi * sinTheta
            );

            SkyVertex vertex = {};
            vertex.position = position;

            vertex.uv = Vec2(u, v);
            vertex.normal = position;
            vertex.tangent = Vec3(
                -sinTheta,
                0.f,
                cosTheta
            );

            vertices.push_back(vertex);
        }
    }

    const uint32 rowSize = slices + 1;

    for (uint32 y = 0; y < stacks; ++y)
    {
        for (uint32 x = 0; x < slices; ++x)
        {
            uint32 a = y * rowSize + x;
            uint32 b = a + rowSize;

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    _vertexBuffer = make_shared<VertexBuffer>();
    _vertexBuffer->Create(vertices);

    _indexBuffer = make_shared<IndexBuffer>();
    _indexBuffer->Create(indices);
}

void Skybox::Render()
{
    if (!_shader || !_material ||
        !_vertexBuffer || !_indexBuffer)
    {
        return;
    }

    // Scene에서 현재 카메라의 Render_Forward() 다음에 호출
    _shader->PushGlobalData(
        Camera::S_MatView,
        Camera::S_MatProjection
    );

    _material->Update();

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

    // 다음 렌더링에 하늘 설정이 남지 않도록 복원
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
    DC->IASetPrimitiveTopology(oldTopology);
}
