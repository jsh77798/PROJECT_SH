#pragma once
#include "Character.h"
#include "ModelAnimation.h"

class EnemyController;

enum class EnemyState : uint8
{
    Idle,
    Move,
    Attack,
    Hit,
    Dead
};

class Enemy : public Character
{
public:
    Enemy();
    virtual ~Enemy();

    virtual void Init() override;
    virtual void Update() override;

    void ChangeState(EnemyState state);

	void Move();
    void Stop();
    void Attack();
    void EndAttack();
    void Death();
    void Hit();

    void SetTarget(shared_ptr<Character> target);

    EnemyState GetState() const { return _state; }

    bool IsDead() const
    {
        return _state == EnemyState::Dead;
    }

    bool IsActionLocked() const
    {
        return _state == EnemyState::Attack ||
            _state == EnemyState::Hit ||
            _state == EnemyState::Dead;
    }

    bool CanAttack() const;

    void SetDamage(float damage) { _damage = damage; }
    float GetDamage() const { return _damage; }

protected:
    bool PlayState(EnemyState state, bool restart = false);
    void CancelAttack();

    virtual void CheckAttackHit();

protected:
    // Dog에서 중복 선언하지 마세요.
    shared_ptr<GameObject> _modelObject;
    shared_ptr<EnemyController> _enemyController;

    unordered_map<EnemyState, string> _animMap;
    shared_ptr<Character> _target;

    EnemyState _state = EnemyState::Idle;
    bool _stateInitialized = false;

    float _previousAttackProgress = 0.f;
    float _attackHitStart = 15.f / 48.f;
    float _attackHitEnd = 23.f / 48.f;

    float _damage = 10.f;

    bool _attackActive = false;
    bool _attackHitApplied = false;

    // 공격 종료 또는 취소 후 대기 시간
    float _attackCooldown = 1.f;
    float _attackCooldownRemaining = 0.f;

    // 공격 판정 구: 캐릭터 원점을 기준으로 월드 단위
    float _hitRadius = 0.7f;
    float _hitForwardOffset = 0.7f;
    float _hitHeightOffset = 0.f;

    bool _hasDeathAnimation = false;
};

