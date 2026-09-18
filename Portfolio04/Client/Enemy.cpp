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

    // Enemy는 make_shared로 생성한 뒤 Init()을 호출해야 합니다.
    weak_ptr<Enemy> weakSelf =
        static_pointer_cast<Enemy>(shared_from_this());

    _health->SetOnHit(
        [weakSelf](
            const Vec3& attackerPosition,
            bool hasAttackerPosition)
        {
            auto enemy = weakSelf.lock();

            if (!enemy)
                return;

            if (hasAttackerPosition)
            {
                enemy->Hit(attackerPosition);
            }
            else
            {
                // 위치 정보가 없으면 기본 앞 피격
                enemy->Hit();
            }
        }
    );

    _health->SetOnDeath(
        [weakSelf]()
        {
            auto enemy = weakSelf.lock();

            if (enemy)
                enemy->Death();
        }
    );
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

    string animationName;

    if (state == EnemyState::Hit &&
        !_currentHitAnimation.empty())
    {
        animationName = _currentHitAnimation;
    }
    // 일어난 상태에서 Idle 요청 → 서 있는 Idle
    else if (state == EnemyState::Idle &&
        _hasAwakened &&
        !_awakeIdleAnimation.empty())
    {
        animationName = _awakeIdleAnimation;
    }
    else
    {
        auto it = _animMap.find(state);

        if (it == _animMap.end())
            return false;

        animationName = it->second;
    }

    animator->Play(animationName);

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

    case EnemyState::WakeUp:
        WakeUp();
        break;

    case EnemyState::LieDown:
        LieDown();
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

void Enemy::WakeUp()
{
    if (IsActionLocked() || !NeedsWakeUp())
        return;

    if (PlayState(EnemyState::WakeUp))
    {
        GetCharacterMovement()->ClearMovementInput();
    }
}

void Enemy::LieDown()
{
    if (IsActionLocked() || !NeedsLieDown())
        return;

    if (PlayState(EnemyState::LieDown))
    {
        GetCharacterMovement()->ClearMovementInput();
    }
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
    //if (IsDead())
    //    return;
    //
    //// 이번 구현에서는 재피격으로 Hit를 계속 재시작하지 않음
    //if (_state == EnemyState::Hit)
    //    return;
    //
    //const bool wasAttacking =
    //    _state == EnemyState::Attack;
    //
    //CancelAttack();
    //
    //if (PlayState(EnemyState::Hit))
    //    return;
    //
    //// 피격 애니메이션이 없어도 공격은 취소
    //if (wasAttacking)
    //{
    //    if (!PlayState(EnemyState::Idle))
    //    {
    //        _state = EnemyState::Idle;
    //        _stateInitialized = false;
    //    }
    //}

    // 공격 위치를 모르면 앞에서 맞은 것으로 처리
    Hit(
        GetTransform()->GetPosition() +
        GetTransform()->GetForward()
    );
}

void Enemy::Hit(const Vec3& attackerPosition)
{
    if (IsDead())
        return;

    // 현재 Hit 도중 다시 맞아도 이번에는 모션을 재시작하지 않음
    // 체력 감소는 공격 측에서 별도로 적용됩니다.
    if (_state == EnemyState::Hit)
        return;

    Vec3 toAttacker =
        attackerPosition - GetTransform()->GetPosition();

    toAttacker.y = 0.f;

    Vec3 forward = GetTransform()->GetForward();
    forward.y = 0.f;

    bool hitFromFront = true;

    if (toAttacker.LengthSquared() > 0.000001f &&
        forward.LengthSquared() > 0.000001f)
    {
        toAttacker.Normalize();
        forward.Normalize();

        hitFromFront = forward.Dot(toAttacker) >= 0.f;
    }

    //_currentHitAnimation = hitFromFront
    //    ? _frontHitAnimation
    //    : _backHitAnimation;
    //
    //// 해당 방향의 모션이 없으면 반대쪽으로 대체
    //if (_currentHitAnimation.empty())
    //{
    //    _currentHitAnimation = hitFromFront
    //        ? _backHitAnimation
    //        : _frontHitAnimation;
    //}

    // 누운 Idle인지 확인
// WakeUp이 없는 일반 적은 제외
    bool isLyingIdle =
        _state == EnemyState::Idle &&
        NeedsWakeUp();
    
    if (_isThanatosis)
    {
        // 누운 피격 등록 누락을 바로 확인
        if (_ThanatosisHitAnimation.empty())
        {
            OutputDebugStringA(
                "Enemy Hit: lying hit animation is missing\n"
            );
            return;
        }

        _currentHitAnimation = _ThanatosisHitAnimation;
    }
    else
    {
        // 서 있을 때는 앞·뒤 피격 선택
        _currentHitAnimation = hitFromFront
            ? _frontHitAnimation
            : _backHitAnimation;

        if (_currentHitAnimation.empty())
        {
            _currentHitAnimation = hitFromFront
                ? _backHitAnimation
                : _frontHitAnimation;
        }
    }

    EnemyState previousState = _state;

    // 애니메이션 등록이 안 됐다면 기존 행동 유지
    if (!PlayState(EnemyState::Hit))
        return;

    _stateBeforeHit = previousState;

    CancelAttack();
    GetCharacterMovement()->ClearMovementInput();
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
            //CUR_SCENE->Remove(shared_from_this());
        }

        return;
    }

    if (!animator)
        return;

    // WakeUp 종료
    if (_state == EnemyState::WakeUp)
    {
        if (animator->IsAnimationFinished())
        {
            _hasAwakened = true;
            _isThanatosis = false;

            PlayState(EnemyState::Move);
        }

        return;
    }

    if (_state == EnemyState::LieDown)
    {
        if (animator->IsAnimationFinished())
        {
            // 반드시 Idle을 재생하기 전에 변경
            _hasAwakened = false;
            _isThanatosis = true;

            PlayState(EnemyState::Idle);
        }

        return;
    }

    // 피격 종료
    if (_state == EnemyState::Hit)
    {
        if (animator->IsAnimationFinished())
        {
            EnemyState nextState = EnemyState::Idle;

            // 피격으로 중단됐던 자세 전환은 처음부터 다시 재생
            if (_stateBeforeHit == EnemyState::WakeUp ||
                _stateBeforeHit == EnemyState::LieDown)
            {
                nextState = _stateBeforeHit;
            }

            _currentHitAnimation.clear();

            if (!PlayState(nextState))
            {
                if (!PlayState(EnemyState::Idle))
                {
                    _state = EnemyState::Idle;
                    _stateInitialized = false;
                }
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

    //health->TakeDamage(_damage);
    health->TakeDamage(
        _damage,
        GetTransform()->GetPosition()
    );
}
