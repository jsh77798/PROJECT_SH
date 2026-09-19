#pragma once
#include "Character.h"
#include "ModelAnimation.h"

class EnemyController;

enum class EnemyState : uint8
{
    Idle,
    WakeUp,
    LieDown,
    Move,
    Attack,
    Hit,
    Thanatosis,
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
    void WakeUp(); //CLD1, CLD2, CLD3...
    void LieDown(); //CLD1, CLD2, CLD3...
    void Attack();
    void EndAttack();
    void Death();
    void Hit();
    void Hit(const Vec3& attackerPosition);

    void SetTarget(shared_ptr<Character> target);

    EnemyState GetState() const { return _state; }

    bool NeedsWakeUp() const
    {
        return !_hasAwakened &&
            _animMap.find(EnemyState::WakeUp) != _animMap.end();
    }

    bool NeedsLieDown() const
    {
        return _hasAwakened &&
            _animMap.find(EnemyState::LieDown) != _animMap.end();
    }

    bool IsDead() const
    {
        return _state == EnemyState::Dead;
    }

    bool IsActionLocked() const
    {
        return _state == EnemyState::Attack ||
            _state == EnemyState::Hit ||
            _state == EnemyState::Dead ||
            _state == EnemyState::WakeUp ||
            _state == EnemyState::LieDown ||
            _state == EnemyState::Thanatosis;
    }

    bool CanAttack() const;

    void SetDamage(float damage) { _damage = damage; }
    float GetDamage() const { return _damage; }

protected:
    bool PlayState(EnemyState state, bool restart = false);
    void CancelAttack();

    virtual void CheckAttackHit();

    bool TryEnterThanatosis();

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
    float _attackCooldown = 0.45f;
    float _attackCooldownRemaining = 0.f;

    // 공격 판정 구: 캐릭터 원점을 기준으로 월드 단위
    float _hitRadius = 0.7f;
    float _hitForwardOffset = 0.7f;
    float _hitHeightOffset = 0.f;

    bool _hasAwakened = false;
    string _awakeIdleAnimation;
    bool _hasDeathAnimation = false;
    bool _isLying = false;
    EnemyState _stateAfterLieDown = EnemyState::Idle;

    float _thanatosisHealthRatio = 0.2f;
    float _thanatosisDuration = 5.f;
    float _thanatosisTimer = 0.f;
    bool _hasUsedThanatosis = false;
    float _thanatosisWakeRange = 5.f;

    // Hit 애니메이션
    string _frontHitAnimation;
    string _backHitAnimation;
    string _ThanatosisHitAnimation;
    string _currentHitAnimation;
    string _thanatosisIdleAnimation;

    // 피격 전에 일어나거나 눕는 중이었는지 기억
    EnemyState _stateBeforeHit = EnemyState::Idle;
};

