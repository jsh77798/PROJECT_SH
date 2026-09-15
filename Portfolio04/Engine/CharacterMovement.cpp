#include "pch.h"
#include "CharacterMovement.h"
#include "BaseCollider.h"

CharacterMovement::CharacterMovement() : Super(ComponentType::CharacterMovement)
{
}

CharacterMovement::~CharacterMovement()
{
}

void CharacterMovement::Update()
{
	TickMovement();
}

void CharacterMovement::AddMovementInput(const Vec3& direction)
{
    _inputVector += direction;
}

void CharacterMovement::Move(const Vec3& delta)
{
    Vec3 horizontalDelta(delta.x, 0.f, delta.z);

    if (horizontalDelta.LengthSquared() < FLT_EPSILON)
        return;

    auto transform = GetTransform();
    auto collider = GetGameObject()->GetCollider();

    auto setPositionAndSync = [&](const Vec3& position)
        {
            transform->SetPosition(position);

            if (collider)
                collider->Update();
        };

    // 이동 성공 시 새 위치 유지
    // 실패 시 시도 직전 위치로 복구
    auto tryMove = [&](const Vec3& movement) -> bool
        {
            if (movement.LengthSquared() < FLT_EPSILON)
                return false;

            const Vec3 startPosition = transform->GetPosition();

            setPositionAndSync(startPosition + movement);

            Vec3 normal = Vec3::Zero;

            if (ResolveCollision(normal))
            {
                setPositionAndSync(startPosition);
                return false;
            }

            return true;
        };

    // 1. 원하는 방향으로 먼저 이동
    if (tryMove(horizontalDelta))
        return;

    // 2. 막히면 X/Z축을 나눠 시도
    const Vec3 moveX(horizontalDelta.x, 0.f, 0.f);
    const Vec3 moveZ(0.f, 0.f, horizontalDelta.z);

    // 이동량이 큰 축부터 처리
    if (fabsf(horizontalDelta.x) >= fabsf(horizontalDelta.z))
    {
        tryMove(moveX);
        tryMove(moveZ);
    }
    else
    {
        tryMove(moveZ);
        tryMove(moveX);
    }
}

void CharacterMovement::RotateTo(const Vec3& direction)
{
    if (direction.LengthSquared() <= 0.001f)
        return;

    Vec3 dir = direction;
    dir.y = 0.f;
    dir.Normalize();

    // 목표 각도
    float targetAngle = atan2f(dir.x, dir.z);

    // 현재 각도
    float currentAngle =
        GetTransform()->GetLocalRotation().y;

    // 현재 -> 목표의 가장 짧은 각도 차이
    float deltaAngle = targetAngle - currentAngle;

    // -PI ~ PI 범위로 정규화
    while (deltaAngle > XM_PI)
        deltaAngle -= XM_2PI;

    while (deltaAngle < -XM_PI)
        deltaAngle += XM_2PI;

    // 이번 프레임에 회전할 수 있는 최대 각도
    float maxAngle =
        _rotationSpeed *
        TIME->GetDeltaTime();

    // 목표까지 남은 각도가 더 작다면 바로 목표까지
    if (fabs(deltaAngle) <= maxAngle)
    {
        currentAngle = targetAngle;
    }
    else
    {
        if (deltaAngle > 0.f)
            currentAngle += maxAngle;
        else
            currentAngle -= maxAngle;
    }

    Vec3 rotation =
        GetTransform()->GetLocalRotation();

    rotation.y = currentAngle;

    GetTransform()->SetLocalRotation(rotation);
}

void CharacterMovement::TickMovement()
{
    if (_inputVector.LengthSquared() == 0.f)
        return;

    _inputVector.Normalize();

    Vec3 delta = _inputVector * _moveSpeed * TIME->GetDeltaTime();

    Move(delta);

    // 다음 프레임을 위해 초기화
    _inputVector = Vec3::Zero;
}

bool CharacterMovement::ResolveCollision(Vec3& normal)
{
    shared_ptr<BaseCollider> collider =
        GetGameObject()->GetCollider();

    if (collider == nullptr)
        return false;

    return SCENE->GetCurrentScene()->CheckCollision(
        collider,
        normal);
}

void CharacterMovement::Slide(Vec3& position)
{
}
