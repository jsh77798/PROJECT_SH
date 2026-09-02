#include "pch.h"
#include "AABBBoxCollider.h"
#include "SphereCollider.h"
#include "OBBBoxCollider.h"
#include "Camera.h"

AABBBoxCollider::AABBBoxCollider(shared_ptr<Shader> shader) 
	: BaseCollider(ColliderType::AABB), _shader(shader)
{
    vector<Vec3> vertices;

    // =========================
    // 8개의 꼭짓점
    // =========================

    Vec3 verticesPos[8] =
    {
        Vec3(-1.f,  1.f, -1.f), // 0
        Vec3(1.f,  1.f, -1.f), // 1
        Vec3(-1.f, -1.f, -1.f), // 2
        Vec3(1.f, -1.f, -1.f), // 3

        Vec3(-1.f,  1.f,  1.f), // 4
        Vec3(1.f,  1.f,  1.f), // 5
        Vec3(-1.f, -1.f,  1.f), // 6
        Vec3(1.f, -1.f,  1.f)  // 7
    };

    // =========================
    // 앞면
    // =========================

    vertices.push_back(verticesPos[0]);
    vertices.push_back(verticesPos[1]);

    vertices.push_back(verticesPos[1]);
    vertices.push_back(verticesPos[3]);

    vertices.push_back(verticesPos[3]);
    vertices.push_back(verticesPos[2]);

    vertices.push_back(verticesPos[2]);
    vertices.push_back(verticesPos[0]);

    // =========================
    // 뒷면
    // =========================

    vertices.push_back(verticesPos[4]);
    vertices.push_back(verticesPos[5]);

    vertices.push_back(verticesPos[5]);
    vertices.push_back(verticesPos[7]);

    vertices.push_back(verticesPos[7]);
    vertices.push_back(verticesPos[6]);

    vertices.push_back(verticesPos[6]);
    vertices.push_back(verticesPos[4]);

    // =========================
    // 앞 <-> 뒤 연결
    // =========================

    vertices.push_back(verticesPos[0]);
    vertices.push_back(verticesPos[4]);

    vertices.push_back(verticesPos[1]);
    vertices.push_back(verticesPos[5]);

    vertices.push_back(verticesPos[2]);
    vertices.push_back(verticesPos[6]);

    vertices.push_back(verticesPos[3]);
    vertices.push_back(verticesPos[7]);

    _vertexCount = static_cast<uint32>(vertices.size());

    _vertexBuffer = make_shared<VertexBuffer>();
    _vertexBuffer->Create(vertices);
}

AABBBoxCollider::~AABBBoxCollider()
{

}

void AABBBoxCollider::Update()
{
    _boundingBox.Center =
        GetGameObject()->GetTransform()->GetPosition();

    _boundingBox.Extents = _extents;
}

bool AABBBoxCollider::Intersects(Ray& ray, OUT float& distance)
{
	return _boundingBox.Intersects(ray.position, ray.direction, OUT distance);
}

bool AABBBoxCollider::Intersects(shared_ptr<BaseCollider>& other)
{
	ColliderType type = other->GetColliderType();

	switch (type)
	{
	case ColliderType::Sphere:
		return _boundingBox.Intersects(dynamic_pointer_cast<SphereCollider>(other)->GetBoundingSphere());
	case ColliderType::AABB:
		return _boundingBox.Intersects(dynamic_pointer_cast<AABBBoxCollider>(other)->GetBoundingBox());
	case ColliderType::OBB:
		return _boundingBox.Intersects(dynamic_pointer_cast<OBBBoxCollider>(other)->GetBoundingBox());
	}
}

bool AABBBoxCollider::GetCollisionNormal(shared_ptr<BaseCollider>& other, OUT Vec3& normal)
{
    if (other == nullptr)
        return false;

    if (other->GetColliderType() != ColliderType::AABB)
        return false;

    auto otherCollider =
        dynamic_pointer_cast<AABBBoxCollider>(other);

    if (otherCollider == nullptr)
        return false;

    BoundingBox& boxA = _boundingBox;
    BoundingBox& boxB = otherCollider->GetBoundingBox();

    Vec3 centerA = boxA.Center;
    Vec3 centerB = boxB.Center;

    Vec3 extentsA = boxA.Extents;
    Vec3 extentsB = boxB.Extents;

    Vec3 distance =
        centerA - centerB;

    // X/Y/Z 방향으로 겹치는 양
    float overlapX =
        extentsA.x + extentsB.x - fabsf(distance.x);

    float overlapY =
        extentsA.y + extentsB.y - fabsf(distance.y);

    float overlapZ =
        extentsA.z + extentsB.z - fabsf(distance.z);

    // 실제로 충돌하지 않음
    if (overlapX <= 0.f ||
        overlapY <= 0.f ||
        overlapZ <= 0.f)
    {
        return false;
    }

    // 가장 적게 겹치는 축을 찾는다.
    if (overlapX < overlapY &&
        overlapX < overlapZ)
    {
        normal = Vec3(
            distance.x > 0.f ? 1.f : -1.f,
            0.f,
            0.f);
    }
    else if (overlapY < overlapZ)
    {
        normal = Vec3(
            0.f,
            distance.y > 0.f ? 1.f : -1.f,
            0.f);
    }
    else
    {
        normal = Vec3(
            0.f,
            0.f,
            distance.z > 0.f ? 1.f : -1.f);
    }

    return true;
}

void AABBBoxCollider::DebugRender()
{
    if (_shader == nullptr)
        return;

    if (_vertexBuffer == nullptr)
        return;

    _shader->PushGlobalData(
        Camera::S_MatView,
        Camera::S_MatProjection);

    TransformDesc desc;

    Matrix scale =
        Matrix::CreateScale(
            _extents.x,
            _extents.y,
            _extents.z);

    Matrix translation =
        Matrix::CreateTranslation(
            _boundingBox.Center);

    desc.W =
        scale *
        translation;

    _shader->PushTransformData(desc);

    _vertexBuffer->PushData();

    DC->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    _shader->Draw(
        0,
        0,
        _vertexCount);
}
