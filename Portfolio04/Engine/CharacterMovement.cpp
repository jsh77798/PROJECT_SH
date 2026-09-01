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
    if (delta.LengthSquared() < FLT_EPSILON)
        return;

    Vec3 oldPosition = GetTransform()->GetPosition();
    Vec3 newPosition = oldPosition + delta;

    GetTransform()->SetPosition(newPosition);

    if (ResolveCollision())
    {
        GetTransform()->SetPosition(oldPosition - delta); //?????
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

bool CharacterMovement::ResolveCollision()
{
    auto myCollider = GetGameObject()->GetCollider();

    if (myCollider == nullptr)
        return false;

    auto objects = CUR_SCENE->GetObjects();

    for (auto& obj : objects)
    {
        if (obj.get() == GetGameObject().get())
            continue;

        auto collider = obj->GetCollider();

        if (collider == nullptr)
            continue;

        if (myCollider->Intersects(collider))
            return true;
    }

    return false;
}

void CharacterMovement::Slide(Vec3& position)
{
}
