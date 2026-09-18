#pragma once
#include "MonoBehaviour.h"
#include "Enemy.h"
#include "ModelAnimator.h"
#include "Character.h"
#include "CharacterMovement.h"


class EnemyController : public MonoBehaviour
{
    using Super = MonoBehaviour;

public:
    virtual void Awake() override;
    virtual void Update() override;

    void SetEnemy(Enemy* enemy);

    void SetTarget(shared_ptr<Character> target)
    {
        _player = target;
    }

private:
    bool CanDetectPlayer();
    bool CanSeePlayer();
    void UpdateLostTarget();
    void UpdateIdle();
    void UpdateChase();
    void UpdateAttack();
    void UpdateDead();

private:
    Enemy* _enemy = nullptr;
    shared_ptr<HealthComponent> _health;
    shared_ptr<Character> _player;

private:
    float _detectRange = 10.0f;
    float _viewHalfAngle = 70.f;
    float _attackRange = 2.0f;
    float _attackFacingAngle = 20.f;
};
