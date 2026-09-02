#pragma once
#include "BaseCollider.h"

class SphereCollider : public BaseCollider
{
public:
	SphereCollider(shared_ptr<Shader> shader);
	virtual ~SphereCollider();

	virtual void Update() override;
	virtual bool Intersects(Ray& ray, OUT float& distance) override;
	virtual bool Intersects(shared_ptr<BaseCollider>& other) override;
	bool GetCollisionNormal(shared_ptr<BaseCollider>& other, OUT Vec3& normal);

	virtual void DebugRender() override;

	void SetRadius(float radius) { _radius = radius; }
	BoundingSphere& GetBoundingSphere() { return _boundingSphere; }

private:
	float _radius = 1.f;
	BoundingSphere _boundingSphere;
	shared_ptr<Shader> _shader;
	shared_ptr<VertexBuffer> _vertexBuffer;
	uint32 _vertexCount = 0;
};
