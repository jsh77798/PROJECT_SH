#pragma once
#include "MonoBehaviour.h"
#include "Enemy.h"
#include "ModelAnimator.h"
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

    void SetTarget(shared_ptr<GameObject> target);

private:
    void UpdateIdle();
    void UpdateChase();
    void UpdateAttack();

private:
    Enemy* _enemy = nullptr;
    shared_ptr<GameObject> _player;
	shared_ptr<CharacterMovement> _movement;
    shared_ptr<ModelAnimator> _animator;

private:
    float _detectRange = 10.0f;
    float _attackRange = 2.0f;
};
