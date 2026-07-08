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
		
		Vec3 forward = _target->GetTransform()->GetForward();
		
		Vec3 desiredPos = targetPos - forward * _offset;
		
		Vec3 currentPos = GetTransform()->GetPosition();
		
		Vec3 newPos = currentPos + (desiredPos - currentPos) * _followSpeed * dt;
		
		GetTransform()->SetPosition(newPos);
		
		GetTransform()->LookAt(targetPos);

		//Vec3 targetPos = _target->GetTransform()->GetPosition();
		//Vec3 forward = _target->GetTransform()->GetForward();
		//
		//Vec3 desiredPos = targetPos - forward * _offset;
		//Vec3 currentPos = GetTransform()->GetPosition();
		//
		//// ⭐ 안정적인 보간
		//float t = 1.0f - expf(-_followSpeed * dt);
		//
		//// 위치
		//Vec3 newPos = XMVectorLerp(currentPos, desiredPos, t);
		//GetTransform()->SetPosition(newPos);
		//
		//// 방향 (부드럽게)
		//Vec3 currentForward = GetTransform()->GetForward();
		//Vec3 targetDir = targetPos - newPos;
		//targetDir.Normalize();
		//Vec3 newForward = XMVectorLerp(currentForward, targetDir, t);

		//GetTransform()->SetForward(newForward);
	}
}
