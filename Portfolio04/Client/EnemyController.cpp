#include "pch.h"
#include "EnemyController.h"
#include "Transform.h"
#include "HealthComponent.h"
#include "BaseCollider.h"
#include "Scene.h"

void EnemyController::SetEnemy(Enemy* enemy)
{
    _enemy = enemy;

    _health = _enemy
        ? _enemy->GetHealthComponent()
        : nullptr;
}

void EnemyController::Awake()
{
    if (_enemy)
    {
        _health = _enemy->GetHealthComponent();
    }
}

void EnemyController::Update()
{
    if (!_enemy)
        return;

    if (!_health)
    {
        _health = _enemy->GetHealthComponent();
    }

    if (!_health)
        return;

    auto movement = _enemy->GetCharacterMovement();

    if (!movement)
        return;

    // 사망 판정은 타깃 유무와 관계없이 먼저 처리
    if (_health->IsDead() || _enemy->IsDead())
    {
        movement->ClearMovementInput();
        UpdateDead();
        return;
    }

    // 공격·피격 중에는 AI가 이동/회전/상태 변경하지 않음
    if (_enemy->IsActionLocked())
    {
        movement->ClearMovementInput();
        return;
    }

    if (!_player)
    {
        //UpdateIdle();
        UpdateLostTarget();
        return;
    }

    auto playerHealth = _player->GetHealthComponent();

    if (!playerHealth || playerHealth->IsDead())
    {
        //UpdateIdle();
        UpdateLostTarget();
        return;
    }

    if (!CanSeePlayer())
    {
        //UpdateIdle();
        UpdateLostTarget();
        return;
    }

    if (_enemy->NeedsWakeUp())
    {
        movement->ClearMovementInput();
        _enemy->WakeUp();
        return;
    }

    Vec3 direction =
        _player->GetTransform()->GetPosition() -
        _enemy->GetTransform()->GetPosition();

    direction.y = 0.f;

    if (direction.LengthSquared() <=
        _attackRange * _attackRange)
    {
        UpdateAttack();
    }
    else
    {
        UpdateChase();
    }
}

bool EnemyController::CanDetectPlayer()
{
    if (!_enemy || !_player)
        return false;

    Vec3 direction =
        _player->GetTransform()->GetPosition() -
        _enemy->GetTransform()->GetPosition();

    direction.y = 0.f;

    return direction.LengthSquared() <=
        _detectRange * _detectRange;
}

bool EnemyController::CanSeePlayer()
{
    if (!CanDetectPlayer())
        return false;

    Vec3 enemyPos =
        _enemy->GetTransform()->GetPosition();

    Vec3 playerPos =
        _player->GetTransform()->GetPosition();

    Vec3 horizontalDirection = playerPos - enemyPos;
    horizontalDirection.y = 0.f;

    // 수평 방향이 있을 때만 시야각 검사
    if (horizontalDirection.LengthSquared() > 0.000001f)
    {
        horizontalDirection.Normalize();

        Vec3 forward =
            _enemy->GetTransform()->GetForward();

        forward.y = 0.f;

        if (forward.LengthSquared() < 0.000001f)
            return false;

        forward.Normalize();

        // 기존과 동일: 정면 기준 좌우 45도
        float cosFov =
            std::cos(XMConvertToRadians(_viewHalfAngle));

        if (forward.Dot(horizontalDirection) < cosFov)
            return false;
    }

    // 레이는 높이 차이를 포함한 실제 방향으로 검사
    Vec3 rayDirection = playerPos - enemyPos;
    float targetDistance = rayDirection.Length();

    if (targetDistance < 0.0001f)
        return true;

    rayDirection /= targetDistance;

    Ray ray;
    ray.position = enemyPos;
    ray.direction = rayDirection;

    shared_ptr<BaseCollider> ignoreCollider =
        _enemy->GetCollider();

    shared_ptr<BaseCollider> hitCollider;
    float hitDistance = 0.f;

    // 카메라용 필터를 사용하지 않음
    bool hit = CUR_SCENE->RayCast(
        ray,
        ignoreCollider,
        hitCollider,
        hitDistance
    );

    if (!hit || !hitCollider)
        return false;

    return hitCollider->GetGameObject() == _player;
}

void EnemyController::UpdateLostTarget()
{
    _enemy->GetCharacterMovement()->ClearMovementInput();

    if (_enemy->NeedsLieDown())
    {
        _enemy->LieDown();
    }
    else
    {
        _enemy->Stop();
    }
}

void EnemyController::UpdateIdle()
{
    _enemy->GetCharacterMovement()->ClearMovementInput();
    _enemy->Stop();
}

void EnemyController::UpdateChase()
{
    Vec3 direction =
        _player->GetTransform()->GetPosition() -
        _enemy->GetTransform()->GetPosition();

    direction.y = 0.f;

    if (direction.LengthSquared() < 0.000001f)
    {
        UpdateIdle();
        return;
    }

    direction.Normalize();

    auto movement = _enemy->GetCharacterMovement();

    movement->AddMovementInput(direction);
    movement->RotateTo(direction);

    _enemy->Move();
}

void EnemyController::UpdateAttack()
{
    auto movement = _enemy->GetCharacterMovement();

    // 공격 범위 안에서는 이동을 멈춤
    movement->ClearMovementInput();

    Vec3 direction =
        _player->GetTransform()->GetPosition() -
        _enemy->GetTransform()->GetPosition();

    direction.y = 0.f;

    if (direction.LengthSquared() > 0.000001f)
    {
        direction.Normalize();

        // 공격을 시작하기 전 타깃을 향해 회전
        movement->RotateTo(direction);

        Vec3 forward =
            _enemy->GetTransform()->GetForward();

        forward.y = 0.f;

        if (forward.LengthSquared() < 0.000001f)
        {
            _enemy->Stop();
            return;
        }

        forward.Normalize();

        float requiredDot = std::cos(
            XMConvertToRadians(_attackFacingAngle)
        );

        // 아직 정면을 향하지 않았다면 대기
        if (forward.Dot(direction) < requiredDot)
        {
            _enemy->Stop();
            return;
        }
    }

    // 쿨다운은 Enemy에서 관리
    if (!_enemy->CanAttack())
    {
        _enemy->Stop();
        return;
    }

    // 애니메이션 시작만 요청
    // 데미지는 Enemy::CheckAttackHit()에서 처리
    _enemy->Attack();
}

void EnemyController::UpdateDead()
{
    _enemy->Death();
}
