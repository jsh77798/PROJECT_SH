#include "pch.h"
#include "EnemyController.h"
#include "Transform.h"

void EnemyController::Awake()
{
    _movement = static_pointer_cast<CharacterMovement>(GetGameObject()->GetFixedComponent(ComponentType::CharacterMovement));
    _animator = GetGameObject()->GetModelAnimator();
}

void EnemyController::Update()
{
    if (_player == nullptr)
        return;

    Vec3 Pos = _enemy->GetTransform()->GetPosition();
    Vec3 playerPos = _player->GetTransform()->GetPosition();

    Vec3 direction = playerPos - Pos;

    direction.y = 0.f;

    float distance = direction.Length();

    if (distance <= _attackRange)
    {
        UpdateAttack();
    }
    else if (distance <= _detectRange)
    {
        UpdateChase();
    }
    else
    {
        UpdateIdle();
    }
}

void EnemyController::SetTarget(shared_ptr<GameObject> target)
{
    _player = target;
}

void EnemyController::UpdateIdle()
{
    _enemy->ChangeState(EnemyState::Idle);
}

void EnemyController::UpdateChase()
{
    Vec3 enemyPos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 direction = playerPos - enemyPos;

    direction.y = 0.f;

    if (direction.LengthSquared() <= 0.001f)
        return;

    direction.Normalize();

    _enemy->GetCharacterMovement()
        ->AddMovementInput(direction);

    // 플레이어 방향으로 회전
    _enemy->GetCharacterMovement()->RotateTo(direction);

    _enemy->ChangeState(EnemyState::Move);
}

void EnemyController::UpdateAttack()
{
    Vec3 enemyPos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 direction = playerPos - enemyPos;

    direction.y = 0.f;

    if (direction.LengthSquared() <= 0.001f)
        return;

    direction.Normalize();

    // 플레이어를 바라본다.
    _enemy->GetCharacterMovement()
        ->RotateTo(direction);

    // 현재는 공격 애니메이션이 없으므로
    // 이동 입력은 넣지 않는다.
}
