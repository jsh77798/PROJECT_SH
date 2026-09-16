#pragma once
#include "BaseCollider.h"

class OBBBoxCollider : public BaseCollider
{
public:
	OBBBoxCollider(shared_ptr<Shader> shader);
	virtual ~OBBBoxCollider();

	virtual void Update() override;
	virtual bool Intersects(Ray& ray, OUT float& distance) override;
	virtual bool Intersects(shared_ptr<BaseCollider>& other) override;

	virtual void DebugRender() override;

	void SetExtents(const Vec3& extents) { _extents = extents; }
	BoundingOrientedBox& GetBoundingBox() { return _boundingBox; }

	void SetWorldBox(const BoundingOrientedBox& box)
	{
		_boundingBox = box;
		_extents = box.Extents;
		_useWorldBox = true;
	}

private:	
	Vec3 _extents = Vec3(1.f);
	BoundingOrientedBox _boundingBox;
	shared_ptr<Shader> _shader;
	shared_ptr<VertexBuffer> _vertexBuffer;
	uint32 _vertexCount = 0;
	bool _useWorldBox = false;
};

