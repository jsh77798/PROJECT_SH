#pragma once
#include "Component.h"

enum class ColliderType
{
	Sphere,
	AABB,
	OBB,
};

class BaseCollider : public Component
{
public:
	BaseCollider(ColliderType colliderType);
	virtual ~BaseCollider();

	virtual bool Intersects(Ray& ray, OUT float& distance) = 0;
	virtual bool Intersects(shared_ptr<BaseCollider>& other) = 0;

	void SetGround(bool ground) { _isGround = ground; }
	bool IsGround() const { return _isGround; }

	void SetBlocksCamera(bool blocks)
	{
		_blocksCamera = blocks;
	}

	bool BlocksCamera() const
	{
		return _blocksCamera;
	}

	ColliderType GetColliderType() { return _colliderType; }

	virtual void DebugRender() {}

protected:
	ColliderType _colliderType;
	bool _isGround = false;
	bool _blocksCamera = false;
};

