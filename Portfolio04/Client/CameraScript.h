#pragma once
#include "MonoBehaviour.h"

class CameraScript : public MonoBehaviour
{
public:
	virtual void Start() override;
	virtual void Update() override;
	void PostLateUpdate() override;

	void SetTarget(shared_ptr<GameObject> target) { _target = target; }

	float _speed = 10.f;

private:
	shared_ptr<GameObject> _target;
	float _offset = 6.f;
	float _followSpeed = 4.f;
	Vec3 _followTargetPos = Vec3(0.f, 0.f, 0.f);
	bool _followInitialized = false;
	float _followYaw = 0.f;
	float _rotationFollowSpeed = 3.f;
};

