#include "pch.h"
#include "OBBBoxCollider.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "Camera.h"

OBBBoxCollider::OBBBoxCollider(shared_ptr<Shader> shader) 
	: BaseCollider(ColliderType::OBB), _shader(shader)
{
    vector<Vec3> vertices;

    Vec3 verticesPos[8] =
    {
        Vec3(-1.f,  1.f, -1.f),
        Vec3(1.f,  1.f, -1.f),
        Vec3(-1.f, -1.f, -1.f),
        Vec3(1.f, -1.f, -1.f),

        Vec3(-1.f,  1.f,  1.f),
        Vec3(1.f,  1.f,  1.f),
        Vec3(-1.f, -1.f,  1.f),
        Vec3(1.f, -1.f,  1.f)
    };

    // 앞면
    vertices.push_back(verticesPos[0]);
    vertices.push_back(verticesPos[1]);

    vertices.push_back(verticesPos[1]);
    vertices.push_back(verticesPos[3]);

    vertices.push_back(verticesPos[3]);
    vertices.push_back(verticesPos[2]);

    vertices.push_back(verticesPos[2]);
    vertices.push_back(verticesPos[0]);

    // 뒷면
    vertices.push_back(verticesPos[4]);
    vertices.push_back(verticesPos[5]);

    vertices.push_back(verticesPos[5]);
    vertices.push_back(verticesPos[7]);

    vertices.push_back(verticesPos[7]);
    vertices.push_back(verticesPos[6]);

    vertices.push_back(verticesPos[6]);
    vertices.push_back(verticesPos[4]);

    // 연결
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

OBBBoxCollider::~OBBBoxCollider()
{

}

void OBBBoxCollider::Update()
{
    if (_useWorldBox)
        return;

    auto transform = GetGameObject()->GetTransform();

    _boundingBox.Center =
        transform->GetPosition();

    _boundingBox.Extents =
        _extents;

    Vec3 rotation =
        transform->GetRotation();

    _boundingBox.Orientation =
        Quaternion::CreateFromYawPitchRoll(
            rotation.y,
            rotation.x,
            rotation.z
        );
}

bool OBBBoxCollider::Intersects(Ray& ray, OUT float& distance)
{
	return _boundingBox.Intersects(ray.position, ray.direction, OUT distance);
}

bool OBBBoxCollider::Intersects(shared_ptr<BaseCollider>& other)
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

	return false;
}

void OBBBoxCollider::DebugRender()
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

    Matrix rotation =
        Matrix::CreateFromQuaternion(
            _boundingBox.Orientation);

    Matrix translation =
        Matrix::CreateTranslation(
            _boundingBox.Center);

    desc.W =
        scale *
        rotation *
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
