#pragma once
#include "BaseCollider.h"


class AABBBoxCollider : public BaseCollider
{
public:
	AABBBoxCollider(shared_ptr<Shader> shader);
	virtual ~AABBBoxCollider();

	virtual void Update() override;
	virtual bool Intersects(Ray& ray, OUT float& distance) override;
	virtual bool Intersects(shared_ptr<BaseCollider>& other) override;
    bool GetCollisionNormal(shared_ptr<BaseCollider>& other,OUT Vec3& normal);

	virtual void DebugRender() override;

	void SetExtents(const Vec3& extents) { _extents = extents; }
	BoundingBox& GetBoundingBox() { return _boundingBox; }

private:
	Vec3 _extents = Vec3(1.f);
	BoundingBox _boundingBox;
	shared_ptr<Shader> _shader;
	shared_ptr<VertexBuffer> _vertexBuffer;
	uint32 _vertexCount = 0;
};

