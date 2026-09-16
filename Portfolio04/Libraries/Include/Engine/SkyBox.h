#pragma once

class Shader;
class Texture;
class Material;
class VertexBuffer;
class IndexBuffer;

class Skybox
{
public:
    void Init(
        shared_ptr<Shader> shader,
        shared_ptr<Texture> texture
    );

    void Render();

private:
    struct SkyVertex
    {
        Vec3 position;
        Vec2 uv;
        Vec3 normal;
        Vec3 tangent;
    };

    shared_ptr<Shader> _shader;
    shared_ptr<Material> _material;
    shared_ptr<VertexBuffer> _vertexBuffer;
    shared_ptr<IndexBuffer> _indexBuffer;
};