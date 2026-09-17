#include "pch.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "ModelAnimator.h"
#include "Transform.h"
#include "Scene.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"
#include "HealthComponent.h"

Enemy::Enemy()
{
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{
    InitCharacter();
}

void Enemy::SetTarget(shared_ptr<Character> target)
{
    _target = target;

    if (_enemyController)
    {
        _enemyController->SetTarget(target);
    }
}

bool Enemy::CanAttack() const
{
    return !IsActionLocked() &&
        _attackCooldownRemaining <= 0.f &&
        _target != nullptr &&
        _target.get() != this;
}

bool Enemy::PlayState(EnemyState state, bool restart)
{
    if (!restart &&
        _stateInitialized &&
        _state == state)
    {
        return true;
    }

    if (!_modelObject)
        return false;

    auto animator = _modelObject->GetModelAnimator();

    if (!animator)
        return false;

    auto it = _animMap.find(state);

    if (it == _animMap.end())
        return false;

    animator->Play(it->second);

    _state = state;
    _stateInitialized = true;

    return true;
}

void Enemy::ChangeState(EnemyState state)
{
    // 공격 준비/정리 과정을 건너뛰지 않도록
    // 외부 상태 요청을 각 함수로 전달
    switch (state)
    {
    case EnemyState::Idle:
        Stop();
        break;

    case EnemyState::Move:
        Move();
        break;

    case EnemyState::Attack:
        Attack();
        break;

    case EnemyState::Hit:
        Hit();
        break;

    case EnemyState::Dead:
        Death();
        break;
    }
}

void Enemy::Move()
{
    if (IsActionLocked())
        return;

    PlayState(EnemyState::Move);
}

void Enemy::Stop()
{
    if (IsActionLocked())
        return;

    PlayState(EnemyState::Idle);
}

void Enemy::Attack()
{
    if (!CanAttack())
        return;

    // 공격 애니메이션이 없으면 시작하지 않음
    if (!PlayState(EnemyState::Attack))
        return;

    _previousAttackProgress = 0.f;
    _attackHitApplied = false;
    _attackActive = true;
}

void Enemy::CancelAttack()
{
    if (_attackActive)
    {
        _attackCooldownRemaining = _attackCooldown;
    }

    _attackActive = false;
    _attackHitApplied = false;
    _previousAttackProgress = 0.f;
}

void Enemy::EndAttack()
{
    if (_state != EnemyState::Attack)
        return;

    CancelAttack();

    // Idle 애니메이션 누락 시에도 논리 상태는 해제
    if (!PlayState(EnemyState::Idle))
    {
        _state = EnemyState::Idle;
        _stateInitialized = false;
    }
}

void Enemy::Hit()
{
    if (IsDead())
        return;

    // 이번 구현에서는 재피격으로 Hit를 계속 재시작하지 않음
    if (_state == EnemyState::Hit)
        return;

    const bool wasAttacking =
        _state == EnemyState::Attack;

    CancelAttack();

    if (PlayState(EnemyState::Hit))
        return;

    // 피격 애니메이션이 없어도 공격은 취소
    if (wasAttacking)
    {
        if (!PlayState(EnemyState::Idle))
        {
            _state = EnemyState::Idle;
            _stateInitialized = false;
        }
    }
}

void Enemy::Death()
{
    if (IsDead())
        return;

    CancelAttack();

    _hasDeathAnimation =
        PlayState(EnemyState::Dead);

    // 사망 애니메이션 유무와 관계없이 사망 상태 확정
    _state = EnemyState::Dead;
    _stateInitialized = true;
}

void Enemy::Update()
{
    float dt = TIME->GetDeltaTime();

    if (_attackCooldownRemaining > 0.f)
    {
        _attackCooldownRemaining -= dt;

        if (_attackCooldownRemaining < 0.f)
            _attackCooldownRemaining = 0.f;
    }

    // 컨트롤러와 모델 자식의 Animator 업데이트
    GameObject::Update();

    auto animator = _modelObject
        ? _modelObject->GetModelAnimator()
        : nullptr;

    // 사망 처리는 공격 상태 검사보다 먼저
    if (_state == EnemyState::Dead)
    {
        if (!_hasDeathAnimation ||
            !animator ||
            animator->IsAnimationFinished())
        {
            CUR_SCENE->Remove(shared_from_this());
        }

        return;
    }

    if (!animator)
        return;

    // 피격 종료
    if (_state == EnemyState::Hit)
    {
        if (animator->IsAnimationFinished())
        {
            if (!PlayState(EnemyState::Idle))
            {
                _state = EnemyState::Idle;
                _stateInitialized = false;
            }
        }

        return;
    }

    if (_state != EnemyState::Attack)
        return;

    float progress = 0.f;

    auto it = _animMap.find(EnemyState::Attack);

    if (it != _animMap.end() &&
        animator->GetAnimationProgress(it->second, progress))
    {
        // 이전~현재 진행 구간이 타격 구간과 겹치는지 확인
        // 큰 DT로 타격 구간을 넘어가도 한 번 검사
        bool crossedHitWindow =
            progress >= _previousAttackProgress &&
            progress >= _attackHitStart &&
            _previousAttackProgress <= _attackHitEnd;

        if (_attackActive &&
            !_attackHitApplied &&
            crossedHitWindow)
        {
            CheckAttackHit();
        }

        _previousAttackProgress = progress;
    }

    // 타격 콜백에서 상태가 바뀌었을 수도 있으므로 재확인
    if (_state == EnemyState::Attack &&
        animator->IsAnimationFinished())
    {
        EndAttack();
    }
}

void Enemy::CheckAttackHit()
{
    if (!_attackActive || _attackHitApplied)
        return;

    // 판정 도중에도 타깃의 수명 유지
    auto target = _target;

    if (!target || target.get() == this)
        return;

    auto collider = target->GetCollider();

    if (!collider)
        return;

    collider->Update();

    Vec3 center =
        GetTransform()->GetPosition();

    center += GetTransform()->GetForward()
        * _hitForwardOffset;

    center.y += _hitHeightOffset;

    BoundingSphere hitSphere;
    hitSphere.Center = center;
    hitSphere.Radius = _hitRadius;

    bool hit = false;

    switch (collider->GetColliderType())
    {
    case ColliderType::Sphere:
    {
        auto sphere =
            dynamic_pointer_cast<SphereCollider>(collider);

        if (sphere)
        {
            hit = hitSphere.Intersects(
                sphere->GetBoundingSphere()
            );
        }
        break;
    }

    case ColliderType::AABB:
    {
        auto box =
            dynamic_pointer_cast<AABBBoxCollider>(collider);

        if (box)
        {
            hit = hitSphere.Intersects(
                box->GetBoundingBox()
            );
        }
        break;
    }

    case ColliderType::OBB:
    {
        auto box =
            dynamic_pointer_cast<OBBBoxCollider>(collider);

        if (box)
        {
            hit = hitSphere.Intersects(
                box->GetBoundingBox()
            );
        }
        break;
    }
    }

    if (!hit)
        return;

    auto health = target->GetHealthComponent();

    if (!health)
        return;

    // TakeDamage에서 다른 처리가 발생하기 전에 기록
    _attackHitApplied = true;

    health->TakeDamage(_damage);
}
