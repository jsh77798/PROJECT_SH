#include "pch.h"
#include "CameraScript.h"
#include "Transform.h"
#include "Scene.h"
#include "BaseCollider.h"

void CameraScript::Start()
{

}

void CameraScript::Update()
{
}

void CameraScript::PostLateUpdate()
{
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

    if (_target == nullptr)
    {
        _followInitialized = false;
        _armInitialized = false;
        return;
    }

    float dt = TIME->GetDeltaTime();

    Vec3 targetPos =
        _target->GetTransform()->GetPosition();

    targetPos.y += _lookHeight;

    // 기존에 바라볼 높이를 더하고 있다면 여기서 유지하세요.

    Vec3 forward =
        _target->GetTransform()->GetForward();

    float targetYaw = std::atan2(forward.x, forward.z);

    if (!_followInitialized)
    {
        _followTargetPos = targetPos;
        _followYaw = targetYaw;
        _followInitialized = true;
        _armInitialized = false;
    }

    // 1. 기존 추적 위치 보간
    float positionT =
        1.f - std::exp(-_followSpeed * dt);

    _followTargetPos +=
        (targetPos - _followTargetPos) * positionT;

    // 2. 기존 추적 회전 보간
    float yawDelta = std::remainder(
        targetYaw - _followYaw,
        XM_2PI
    );

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

    // 3. 기존 방식으로 원하는 카메라 위치 계산
    //Vec3 desiredPos = _followTargetPos - followForward * _offset;
    Vec3 desiredPos =
        _followTargetPos
        - followForward * _offset
        + Vec3(0.f, _cameraHeight, 0.f);

    // 4. 현재 캐릭터 위치에서 원하는 카메라 위치까지 검사
    Vec3 armDirection = desiredPos - targetPos;
    float desiredLength = armDirection.Length();

    if (desiredLength < 0.0001f)
    {
        GetTransform()->SetPosition(targetPos);
        _currentArmLength = 0.f;
        _armInitialized = false;
        return;
    }

    armDirection /= desiredLength;

    Ray ray;
    ray.position = targetPos;
    ray.direction = armDirection;

    shared_ptr<BaseCollider> ignoreCollider =
        _target->GetCollider();

    shared_ptr<BaseCollider> hitCollider;
    float hitDistance = 0.f;

    float allowedLength = desiredLength;

    bool hit = CUR_SCENE->RayCast(
        ray,
        ignoreCollider,
        hitCollider,
        hitDistance,
        true
    );

    // 카메라보다 뒤쪽에 있는 벽은 무시
    if (hit && hitDistance <= desiredLength)
    {
        allowedLength =
            hitDistance - _cameraWallMargin;

        if (allowedLength < 0.f)
            allowedLength = 0.f;
    }

    // 5. 가까워질 때 즉시 줄이고, 멀어질 때 부드럽게 복귀
    if (!_armInitialized)
    {
        _currentArmLength = allowedLength;
        _armInitialized = true;
    }
    else if (allowedLength < _currentArmLength)
    {
        _currentArmLength = allowedLength;
    }
    else
    {
        float returnT =
            1.f - std::exp(-_cameraReturnSpeed * dt);

        _currentArmLength +=
            (allowedLength - _currentArmLength) * returnT;
    }

    Vec3 cameraPos =
        targetPos + armDirection * _currentArmLength;

    GetTransform()->SetPosition(cameraPos);

    // 6. 평상시에는 기존 시선 기준 유지
    // 카메라가 가까워지면 현재 캐릭터 쪽으로 시선 기준 이동
    float armRatio =
        _currentArmLength / desiredLength;

    Vec3 lookTarget =
        targetPos +
        (_followTargetPos - targetPos) * armRatio;

    // 두 점이 겹치면 LookAt 호출 생략
    if ((lookTarget - cameraPos).LengthSquared() > 0.000001f)
    {
        GetTransform()->LookAt(lookTarget);
    }
}


