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

    void SetEnemy(Enemy* Enemy)
    {
        _enemy = Enemy;
    }

    void SetTarget(shared_ptr<Character> target)
    {
        _player = target;
    }

private:
    bool CanDetectPlayer();
    bool CanSeePlayer();
    void UpdateIdle();
    void UpdateChase();
    void UpdateAttack();
    void UpdateDead();

    void Attack();

private:
    Enemy* _enemy = nullptr;
    shared_ptr<Character> _player;

private:
    float _detectRange = 10.0f;
    float _attackRange = 2.0f;
    float _attackCooldown = 2.f;
    float _attackTimer = 0.f;
	bool _isDead = false;
};
