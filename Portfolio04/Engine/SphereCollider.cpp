#include "pch.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "Camera.h"

SphereCollider::SphereCollider(shared_ptr<Shader> shader)
	: BaseCollider(ColliderType::Sphere), _shader(shader)
{
    vector<Vec3> vertices;

    const int32 segments = 32;

    for (int32 i = 0; i < segments; i++)
    {
        float a0 = XM_2PI * i / segments;
        float a1 = XM_2PI * (i + 1) / segments;

        // =========================
        // XY Plane
        // =========================

        vertices.push_back(
            Vec3(
                cosf(a0),
                sinf(a0),
                0.f));

        vertices.push_back(
            Vec3(
                cosf(a1),
                sinf(a1),
                0.f));


        // =========================
        // XZ Plane
        // =========================

        vertices.push_back(
            Vec3(
                cosf(a0),
                0.f,
                sinf(a0)));

        vertices.push_back(
            Vec3(
                cosf(a1),
                0.f,
                sinf(a1)));


        // =========================
        // YZ Plane
        // =========================

        vertices.push_back(
            Vec3(
                0.f,
                cosf(a0),
                sinf(a0)));

        vertices.push_back(
            Vec3(
                0.f,
                cosf(a1),
                sinf(a1)));
    }

    _vertexCount = static_cast<uint32>(vertices.size());

    _vertexBuffer = make_shared<VertexBuffer>();
    _vertexBuffer->Create(vertices);
}

SphereCollider::~SphereCollider()
{

}

void SphereCollider::Update()
{
	_boundingSphere.Center = GetGameObject()->GetTransform()->GetPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetScale();
	_boundingSphere.Radius = _radius * max(max(scale.x, scale.y), scale.z);
}

bool SphereCollider::Intersects(Ray& ray, OUT float& distance)
{
	return _boundingSphere.Intersects(ray.position, ray.direction, OUT distance);
}

bool SphereCollider::Intersects(shared_ptr<BaseCollider>& other)
{
	ColliderType type = other->GetColliderType();

	switch (type)
	{
		case ColliderType::Sphere:
			return _boundingSphere.Intersects(dynamic_pointer_cast<SphereCollider>(other)->GetBoundingSphere());
		case ColliderType::AABB:
			return _boundingSphere.Intersects(dynamic_pointer_cast<AABBBoxCollider>(other)->GetBoundingBox());
		case ColliderType::OBB:
			return _boundingSphere.Intersects(dynamic_pointer_cast<OBBBoxCollider>(other)->GetBoundingBox());
	}

	return false;
}

void SphereCollider::DebugRender()
{
    OutputDebugStringA("SphereCollider DebugRender\n");

    Vec3 pos = GetTransform()->GetPosition();

    char buffer[256];
    sprintf_s(
        buffer,
        "Collider : %f %f %f\n",
        pos.x, pos.y, pos.z
    );

    OutputDebugStringA(buffer);







    if (_shader == nullptr)
        return;

    if (_vertexBuffer == nullptr)
        return;

    _shader->PushGlobalData(
        Camera::S_MatView,
        Camera::S_MatProjection);

    TransformDesc desc;

    Matrix scale =
        Matrix::CreateScale(_radius);

    desc.W =
        scale * GetTransform()->GetWorldMatrix();

    _shader->PushTransformData(desc);

    _vertexBuffer->PushData();

    // ★ 디버그 렌더링은 선으로 그린다.
    DC->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    _shader->Draw(
        0,
        0,
        _vertexCount);
}
