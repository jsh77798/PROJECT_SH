#include "pch.h"
#include "CameraScript.h"
#include "Transform.h"

void CameraScript::Start()
{

}

void CameraScript::Update()
{
	//if (_target == nullptr)
	//	return;
	//
	//float dt = TIME->GetDeltaTime();
	//
	//Vec3 targetPos =
	//	_target->GetTransform()->GetPosition();
	//
	//Vec3 forward =
	//	_target->GetTransform()->GetForward();
	//
	//Vec3 desiredPos =
	//	targetPos - forward * _offset;
	//
	//Vec3 currentPos =
	//	GetTransform()->GetPosition();
	//
	//// _followSpeed는 양수로 설정
	//float t = 1.f - std::exp(-_followSpeed * dt);
	//
	//Vec3 newPos =
	//	currentPos + (desiredPos - currentPos) * t;
	//
	//GetTransform()->SetPosition(newPos);
	//GetTransform()->LookAt(targetPos);





    //if (_target == nullptr)
    //{
    //    _followInitialized = false;
    //    return;
    //}
    //
    //float dt = TIME->GetDeltaTime();
    //
    //Vec3 targetPos =
    //    _target->GetTransform()->GetPosition();
    //
    //Vec3 forward =
    //    _target->GetTransform()->GetForward();
    //
    //float targetYaw = std::atan2(forward.x, forward.z);
    //
    //if (!_followInitialized)
    //{
    //    _followTargetPos = targetPos;
    //    _followYaw = targetYaw;
    //    _followInitialized = true;
    //}
    //
    //// 1. 추적 위치 보간
    //float positionT =
    //    1.f - std::exp(-_followSpeed * dt);
    //
    //_followTargetPos +=
    //    (targetPos - _followTargetPos) * positionT;
    //
    //// 2. 회전 차이를 -PI ~ PI 범위로 변환
    //// 179도 → -179도에서도 짧은 방향으로 회전
    //float yawDelta = std::remainder(
    //    targetYaw - _followYaw,
    //    XM_2PI
    //);
    //
    //// 3. 추적 방향 보간
    //float rotationT =
    //    1.f - std::exp(-_rotationFollowSpeed * dt);
    //
    //_followYaw = std::remainder(
    //    _followYaw + yawDelta * rotationT,
    //    XM_2PI
    //);
    //
    //Vec3 followForward(
    //    std::sin(_followYaw),
    //    0.f,
    //    std::cos(_followYaw)
    //);
    //
    //// 4. 같은 추적 기준점으로 위치와 시선 계산
    //Vec3 cameraPos =
    //    _followTargetPos - followForward * _offset;
    //
    //GetTransform()->SetPosition(cameraPos);
    //GetTransform()->LookAt(_followTargetPos);
}

void CameraScript::PostLateUpdate()
{
    if (_target == nullptr)
    {
        _followInitialized = false;
        return;
    }

    float dt = TIME->GetDeltaTime();

    Vec3 targetPos =
        _target->GetTransform()->GetPosition();

    Vec3 forward =
        _target->GetTransform()->GetForward();

    float targetYaw = std::atan2(forward.x, forward.z);

    if (!_followInitialized)
    {
        _followTargetPos = targetPos;
        _followYaw = targetYaw;
        _followInitialized = true;
    }

    // 1. 추적 위치 보간
    float positionT =
        1.f - std::exp(-_followSpeed * dt);

    _followTargetPos +=
        (targetPos - _followTargetPos) * positionT;

    // 2. 회전 차이를 -PI ~ PI 범위로 변환
    // 179도 → -179도에서도 짧은 방향으로 회전
    float yawDelta = std::remainder(
        targetYaw - _followYaw,
        XM_2PI
    );

    // 3. 추적 방향 보간
    float rotationT =
        1.f - std::exp(-_rotationFollowSpeed * dt);

    _followYaw = std::remainder(
        _followYaw + yawDelta * rotationT,
        XM_2PI
    );

    Vec3 followForward(
        std::sin(_followYaw),
        0.f,
        std::cos(_followYaw)
    );

    // 4. 같은 추적 기준점으로 위치와 시선 계산
    Vec3 cameraPos =
        _followTargetPos - followForward * _offset;

    GetTransform()->SetPosition(cameraPos);
    GetTransform()->LookAt(_followTargetPos);
}


