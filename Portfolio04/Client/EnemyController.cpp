#include "pch.h"
#include "EnemyController.h"
#include "Transform.h"
#include "HealthComponent.h"

void EnemyController::Awake()
{
    _health = _enemy->GetHealthComponent();
}

void EnemyController::Update()
{

    if (_health == nullptr)
        return;

    if (_health->IsDead())
    {
        UpdateDead();
        return;
    }

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

bool EnemyController::CanDetectPlayer()
{
    Vec3 Pos = _enemy->GetTransform()->GetPosition();
    Vec3 playerPos = _player->GetTransform()->GetPosition();

    Vec3 direction = playerPos - Pos;

    direction.y = 0.f;

    float distanceSquared =
        direction.LengthSquared();

    return distanceSquared <=
        _detectRange * _detectRange;
}

bool EnemyController::CanSeePlayer()
{
    // 1. 거리
    if (CanDetectPlayer() == false)
        return false;

    Vec3 enemyPos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 direction =
        playerPos - enemyPos;

    direction.y = 0.f;

    if (direction.LengthSquared() <= 0.001f)
        return true;

    direction.Normalize();

    // 2. FOV
    Vec3 forward =
        _enemy->GetTransform()->GetForward();

    forward.y = 0.f;
    forward.Normalize();

    float dot =
        forward.Dot(direction);

    float cosFov =
        cosf(XMConvertToRadians(45.f));

    if (dot < cosFov)
        return false;

    // 3. RayCast
    Ray ray;

    ray.position = enemyPos;
    ray.direction = direction;

    shared_ptr<BaseCollider> enemyCollider =
        _enemy->GetCollider();

    shared_ptr<BaseCollider> hitCollider;

    float hitDistance = 0.f;

    if (SCENE->GetCurrentScene()->RayCast(
        ray,
        enemyCollider,
        hitCollider,
        hitDistance) == false)
    {
        return false;
    }

    // 4. 가장 먼저 맞은 Collider가 Player인지 확인
    if (hitCollider->GetGameObject() != _player)
        return false;

    return true;
}

void EnemyController::UpdateIdle()
{
    if (_health == nullptr)
        return;

    if (_health->IsDead())
    {
        UpdateDead();
        return;
    }

    _enemy->ChangeState(EnemyState::Idle);
}

void EnemyController::UpdateChase()
{
    if (_health == nullptr)
        return;

    if (_health->IsDead())
    {
        UpdateDead();
        return;
    }

    if (CanSeePlayer() == false)
    {
        _enemy->ChangeState(EnemyState::Idle);
        return;
    }

    Vec3 Pos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 direction =
        playerPos - Pos;

    direction.y = 0.f;

    if (direction.LengthSquared() <= 0.001f)
        return;

    direction.Normalize();

    _enemy->GetCharacterMovement()
        ->AddMovementInput(direction);

    _enemy->GetCharacterMovement()
        ->RotateTo(direction);

    _enemy->ChangeState(
        EnemyState::Move);
}

void EnemyController::UpdateAttack()
{
    if (_health == nullptr)
        return;

    if (_health->IsDead())
    {
        UpdateDead();
        return;
    }

    if (CanSeePlayer() == false)
        return;

    Vec3 enemyPos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 direction =
        playerPos - enemyPos;

    direction.y = 0.f;

    float distanceSquared =
        direction.LengthSquared();

    // 공격 범위를 벗어나면 다시 추적
    if (distanceSquared > _attackRange * _attackRange)
    {
        _enemy->ChangeState(EnemyState::Idle);
        return;
    }

    // Player 방향 바라보기
    if (direction.LengthSquared() > 0.001f)
    {
        direction.Normalize();

        _enemy->GetCharacterMovement()
            ->RotateTo(direction);
    }

    // 공격 쿨타임
    _attackTimer -= TIME->GetDeltaTime();

    if (_attackTimer <= 0.f)
    {
        Attack();

        _attackTimer = _attackCooldown;
    }
}

void EnemyController::UpdateDead()
{
    if (_isDead)
        return;

    _isDead = true;

	_enemy->Death();
}

void EnemyController::Attack()
{
    if (_player == nullptr)
        return;

	_enemy->ChangeState(EnemyState::Attack);

    auto health =
		_player->GetHealthComponent();

    if (health == nullptr)
        return;

    health->TakeDamage(100.f);
}
