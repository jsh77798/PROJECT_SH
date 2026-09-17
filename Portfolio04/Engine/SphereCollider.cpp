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
    //auto transform = GetGameObject()->GetTransform();
    //
    //Vec3 scale = transform->GetScale();
    //
    //float scaleMax = max(
    //    max(fabsf(scale.x), fabsf(scale.y)),
    //    fabsf(scale.z)
    //);
    //
    //_boundingSphere.Radius = _radius * scaleMax;
    //
    //Vec3 center = transform->GetPosition();
    //
    //if (_useFootPosition)
    //{
    //    center.y += _boundingSphere.Radius;
    //}
    //
    //_boundingSphere.Center = center;

    auto transform = GetGameObject()->GetTransform();

    // 반지름 계산
    Vec3 scale = transform->GetScale();

    float scaleMax = fabsf(scale.x);

    if (fabsf(scale.y) > scaleMax)
        scaleMax = fabsf(scale.y);

    if (fabsf(scale.z) > scaleMax)
        scaleMax = fabsf(scale.z);

    _boundingSphere.Radius = _radius * scaleMax;

    // 로컬 오프셋을 월드 좌표로 변환
    Vec3 center = XMVector3TransformCoord(
        _centerOffset,
        transform->GetWorldMatrix()
    );

    // 기존 발 기준 모드를 사용하는 경우에만 추가
    if (_useFootPosition)
    {
        center.y += _boundingSphere.Radius;
    }

    _boundingSphere.Center = center;
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

bool SphereCollider::GetCollisionNormal(shared_ptr<BaseCollider>& other, OUT Vec3& normal)
{
    if (other == nullptr)
        return false;

    // Sphere <-> Sphere
    if (other->GetColliderType() == ColliderType::Sphere)
    {
        auto sphere =
            dynamic_pointer_cast<SphereCollider>(other);

        if (sphere == nullptr)
            return false;

        Vec3 centerA =
            _boundingSphere.Center;

        Vec3 centerB =
            sphere->GetBoundingSphere().Center;

        normal = centerA - centerB;
        normal.y = 0.f;

        if (normal.LengthSquared() < FLT_EPSILON)
            return false;

        normal.Normalize();

        return true;
    }

    // Sphere <-> AABB
    if (other->GetColliderType() == ColliderType::AABB)
    {
        auto aabb =
            dynamic_pointer_cast<AABBBoxCollider>(other);

        if (aabb == nullptr)
            return false;

        BoundingSphere& sphere =
            _boundingSphere;

        BoundingBox& box =
            aabb->GetBoundingBox();

        Vec3 sphereCenter = sphere.Center;
        Vec3 boxCenter = box.Center;
        Vec3 boxExtents = box.Extents;

        Vec3 closestPoint;

        closestPoint.x =
            max(boxCenter.x - boxExtents.x,
                min(sphereCenter.x,
                    boxCenter.x + boxExtents.x));

        closestPoint.y =
            max(boxCenter.y - boxExtents.y,
                min(sphereCenter.y,
                    boxCenter.y + boxExtents.y));

        closestPoint.z =
            max(boxCenter.z - boxExtents.z,
                min(sphereCenter.z,
                    boxCenter.z + boxExtents.z));

        normal =
            sphereCenter - closestPoint;

        normal.y = 0.f;

        if (normal.LengthSquared() < FLT_EPSILON)
            return false;

        normal.Normalize();

        return true;
    }

    return false;
}

void SphereCollider::DebugRender()
{
	// 디버그 렌더링을 위한 로그 출력
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
    }

    //if (_shader == nullptr)
    //    return;
    //
    //if (_vertexBuffer == nullptr)
    //    return;
    //
    //_shader->PushGlobalData(
    //    Camera::S_MatView,
    //    Camera::S_MatProjection);
    //
    //TransformDesc desc;
    //
    //Matrix scale =
    //    Matrix::CreateScale(_radius);
    //
    //desc.W =
    //    scale * GetTransform()->GetWorldMatrix();
    //
    //_shader->PushTransformData(desc);
    //
    //_vertexBuffer->PushData();
    //
    //// ★ 디버그 렌더링은 선으로 그린다.
    //DC->IASetPrimitiveTopology(
    //    D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    //
    //_shader->Draw(
    //    0,
    //    0,
    //    _vertexCount);



    if (_shader == nullptr || _vertexBuffer == nullptr)
        return;

    _shader->PushGlobalData(
        Camera::S_MatView,
        Camera::S_MatProjection
    );

    // 실제 충돌 검사에 사용하는 중심과 반지름
    Vec3 center = _boundingSphere.Center;
    float radius = _boundingSphere.Radius;

    Matrix scale = Matrix::CreateScale(radius);

    Matrix translation =
        Matrix::CreateTranslation(center);

    TransformDesc desc;
    desc.W = scale * translation;

    _shader->PushTransformData(desc);

    _vertexBuffer->PushData();

    DC->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST
    );

    _shader->Draw(0, 0, _vertexCount);
}
