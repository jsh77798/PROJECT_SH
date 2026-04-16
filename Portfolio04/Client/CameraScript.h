#pragma once
#include "MonoBehaviour.h"

class CameraScript : public MonoBehaviour
{
public:
	virtual void Start() override;
	virtual void Update() override;

	void SetTarget(shared_ptr<GameObject> target) { _target = target; }

	float _speed = 10.f;

private:
	shared_ptr<GameObject> _target;
	float _offset = 6.f;
	float _followSpeed = 5.f;
};

