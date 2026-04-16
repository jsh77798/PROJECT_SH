#include "pch.h"
#include "CameraScript.h"
#include "Transform.h"

void CameraScript::Start()
{

}

void CameraScript::Update()
{
	float dt = TIME->GetDeltaTime();

	if (_target == nullptr)
		return;
	// 카메라가 타겟을 따라가도록 구현
	{
		Vec3 targetPos = _target->GetTransform()->GetPosition();  

		Vec3 forward = _target->GetTransform()->GetLook();

		Vec3 desiredPos = targetPos - forward * _offset;

		Vec3 currentPos = GetTransform()->GetPosition();

		Vec3 newPos = currentPos + (desiredPos - currentPos) * _followSpeed * dt;

		GetTransform()->SetPosition(newPos);

		GetTransform()->LookAt(targetPos);
	}
}
