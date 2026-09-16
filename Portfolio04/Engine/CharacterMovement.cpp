#include "pch.h"
#include "CharacterMovement.h"
#include "BaseCollider.h"
#include "Scene.h"
#include "GameObject.h"
#include "Transform.h"

CharacterMovement::CharacterMovement() : Super(ComponentType::CharacterMovement)
{
}

CharacterMovement::~CharacterMovement()
{
}

void CharacterMovement::Update()
{
	//TickMovement();
}

void CharacterMovement::LateUpdate()
{
    TickMovement();
}

void CharacterMovement::AddMovementInput(const Vec3& direction)
{
    _inputVector += direction;
}

void CharacterMovement::Move(const Vec3& delta)
{
    //Vec3 horizontalDelta(delta.x, 0.f, delta.z);
    //
    //if (horizontalDelta.LengthSquared() < FLT_EPSILON)
    //    return;
    //
    //auto transform = GetTransform();
    //auto collider = GetGameObject()->GetCollider();
    //
    //auto setPositionAndSync = [&](const Vec3& position)
    //    {
    //        transform->SetPosition(position);
    //
    //        if (collider)
    //            collider->Update();
    //    };
    //
    //// 이동 성공 시 새 위치 유지
    //// 실패 시 시도 직전 위치로 복구
    //auto tryMove = [&](const Vec3& movement) -> bool
    //    {
    //        if (movement.LengthSquared() < FLT_EPSILON)
    //            return false;
    //
    //        const Vec3 startPosition = transform->GetPosition();
    //
    //        setPositionAndSync(startPosition + movement);
    //
    //        Vec3 normal = Vec3::Zero;
    //
    //        if (ResolveCollision(normal))
    //        {
    //            setPositionAndSync(startPosition);
    //            return false;
    //        }
    //
    //        return true;
    //    };
    //
    //// 1. 원하는 방향으로 먼저 이동
    //if (tryMove(horizontalDelta))
    //    return;
    //
    //// 2. 막히면 X/Z축을 나눠 시도
    //const Vec3 moveX(horizontalDelta.x, 0.f, 0.f);
    //const Vec3 moveZ(0.f, 0.f, horizontalDelta.z);
    //
    //// 이동량이 큰 축부터 처리
    //if (fabsf(horizontalDelta.x) >= fabsf(horizontalDelta.z))
    //{
    //    tryMove(moveX);
    //    tryMove(moveZ);
    //}
    //else
    //{
    //    tryMove(moveZ);
    //    tryMove(moveX);
    //}

    Vec3 horizontalDelta(delta.x, 0.f, delta.z);

    if (horizontalDelta.LengthSquared() < FLT_EPSILON)
        return;

    if (TryGroundMove(horizontalDelta))
        return;

    Vec3 moveX(horizontalDelta.x, 0.f, 0.f);
    Vec3 moveZ(0.f, 0.f, horizontalDelta.z);

    if (fabsf(horizontalDelta.x) >= fabsf(horizontalDelta.z))
    {
        TryGroundMove(moveX);
        TryGroundMove(moveZ);
    }
    else
    {
        TryGroundMove(moveZ);
        TryGroundMove(moveX);
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
    //if (_inputVector.LengthSquared() == 0.f)
    //    return;
    //
    //_inputVector.Normalize();
    //
    //Vec3 delta = _inputVector * _moveSpeed * TIME->GetDeltaTime();
    //
    //Move(delta);
    //
    //// 다음 프레임을 위해 초기화
    //_inputVector = Vec3::Zero;

    const float dt = TIME->GetDeltaTime();

    Vec3 input = _inputVector;
    _inputVector = Vec3::Zero;

    if (dt <= 0.f)
        return;

    input.y = 0.f;

    // 수평 이동
    if (input.LengthSquared() >= FLT_EPSILON)
    {
        input.Normalize();

        Vec3 delta = input * _moveSpeed * dt;
        Move(delta);
    }

    // 입력이 없어도 중력과 착지는 항상 처리
    TickVertical(dt);
}

void CharacterMovement::TickVertical(float dt)
{
    if (dt <= 0.f)
        return;

    Vec3 position = GetTransform()->GetPosition();

    const float footY =
        position.y - _footOffset;

    float groundY = 0.f;

    // 1. 접지 중이면 가까운 아래쪽 바닥을 계속 따라감
    if (_isGrounded)
    {
        bool hasGround = FindGroundInRange(
            position,
            footY - _groundSnapDistance,
            footY + 0.01f,
            groundY
        );

        if (hasGround)
        {
            Vec3 snappedPosition = position;

            snappedPosition.y =
                groundY + _footOffset + _groundOffset;

            SetPositionAndSync(snappedPosition);

            if (!HasBlockingCollision())
            {
                _verticalVelocity = 0.f;
                return;
            }

            SetPositionAndSync(position);
        }

        // 지지할 바닥이 사라짐
        _isGrounded = false;
        _verticalVelocity = 0.f;
    }

    // 2. 중력으로 수직 속도 변경
    _verticalVelocity -= _gravity * dt;

    if (_verticalVelocity < -_maxFallSpeed)
        _verticalVelocity = -_maxFallSpeed;

    const float nextCenterY =
        position.y + _verticalVelocity * dt;

    const float nextFootY =
        nextCenterY - _footOffset;

    // 3. 이번 프레임에 발이 지나갈 전체 높이 구간 검사
    // groundOffset만큼 바닥 위에서 착지하도록 범위 보정
    bool landed = FindGroundInRange(
        position,
        nextFootY - _groundOffset,
        footY,
        groundY
    );

    if (landed)
    {
        // 4. 착지
        position.y =
            groundY + _footOffset + _groundOffset;

        SetPositionAndSync(position);

        _verticalVelocity = 0.f;
        _isGrounded = true;
        return;
    }

    // 5. 바닥이 없으면 계속 낙하
    position.y = nextCenterY;
    SetPositionAndSync(position);
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

void CharacterMovement::SetPositionAndSync(const Vec3& position)
{
    GetTransform()->SetPosition(position);

    auto collider = GetGameObject()->GetCollider();

    if (collider)
        collider->Update();
}

bool CharacterMovement::FindGround(const Vec3& position, float referenceFootY, float& groundY)
{
    return FindGroundInRange(
        position,
        referenceFootY - _groundSnapDistance,
        referenceFootY + _stepHeight,
        groundY
    );
}

bool CharacterMovement::FindGroundInRange(const Vec3& position, float minGroundY, float maxGroundY, float& groundY)
{
    if (minGroundY > maxGroundY)
        return false;

    constexpr float probeMargin = 0.01f;
    constexpr float tolerance = 0.0001f;

    Ray ray;

    // 검사할 높이 범위보다 조금 위에서 아래로 검사
    ray.position = Vec3(
        position.x,
        maxGroundY + probeMargin,
        position.z
    );

    ray.direction = Vec3(0.f, -1.f, 0.f);

    const float maxDistance =
        ray.position.y - minGroundY;

    float nearestDistance = FLT_MAX;
    bool found = false;

    auto selfCollider = GetGameObject()->GetCollider();

    for (const auto& object :
        SCENE->GetCurrentScene()->GetObjects())
    {
        auto other = object->GetCollider();

        if (other == nullptr ||
            other == selfCollider ||
            !other->IsGround())
        {
            continue;
        }

        other->Update();

        float distance = 0.f;

        if (!other->Intersects(ray, distance))
            continue;

        if (distance < 0.f ||
            distance > maxDistance + tolerance)
        {
            continue;
        }

        float hitY = ray.position.y - distance;

        // 유효 범위인지 먼저 검사한 뒤 가장 가까운 바닥 선택
        if (hitY < minGroundY - tolerance ||
            hitY > maxGroundY + tolerance)
        {
            continue;
        }

        if (distance >= nearestDistance)
            continue;

        nearestDistance = distance;
        groundY = hitY;
        found = true;
    }

    return found;
}

bool CharacterMovement::HasBlockingCollision()
{
    auto collider = GetGameObject()->GetCollider();

    if (collider == nullptr)
        return false;

    for (const auto& object :
        SCENE->GetCurrentScene()->GetObjects())
    {
        auto other = object->GetCollider();

        if (other == nullptr ||
            other == collider ||
            other->IsGround())
        {
            continue;
        }

        other->Update();

        if (collider->Intersects(other))
        {
           
            return true;
        }
    }

    return false;
}

bool CharacterMovement::TryGroundMove(const Vec3& movement)
{
    if (movement.LengthSquared() < FLT_EPSILON)
        return false;

    const Vec3 startPosition =
        GetTransform()->GetPosition();

    Vec3 nextPosition = startPosition + movement;

    // 바닥에 서 있을 때만 단차와 경사면을 따라 높이 조정
    if (_isGrounded)
    {
        const float footY =
            startPosition.y - _footOffset;

        float groundY = 0.f;

        if (FindGround(nextPosition, footY, groundY))
        {
            nextPosition.y =
                groundY + _footOffset + _groundOffset;
        }

        // 바닥을 못 찾았으면 기존 Y를 유지하고 이동
        // 낙하는 TickVertical()에서 처리
    }

    SetPositionAndSync(nextPosition);

    if (HasBlockingCollision())
    {
        SetPositionAndSync(startPosition);
        return false;
    }

    return true;
}
