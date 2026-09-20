#pragma once

#include "GameObject.h"

class Shader;
class VertexBuffer;
class IndexBuffer;

struct VertexBlood
{
    Vec3 position; // 생성 위치: 월드 좌표
    Vec2 uv;
    Vec2 scale;
    Vec3 velocity;
    float lifetime;
};

class BloodEffect : public GameObject
{
public:
    void Init(
        shared_ptr<Shader> shader,
        const Vec3& position,
        const Vec3& direction);

    void Update() override;
    void Render();

private:
    shared_ptr<Shader> _shader;
    shared_ptr<VertexBuffer> _vertexBuffer;
    shared_ptr<IndexBuffer> _indexBuffer;

    float _elapsedTime = 0.f;
    float _duration = 0.45f;
};