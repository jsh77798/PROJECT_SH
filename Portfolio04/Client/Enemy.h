#pragma once
#include "Character.h"
#include "GameObject.h"
#include "ModelAnimation.h"

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

public:
    virtual void Init() override;
    virtual void Update() override;

    void ChangeState(EnemyState state);

	void Move();
    void Stop();
    void Attack();
    void Death();

    void SetTarget(shared_ptr<Character> target)
    {
        _target = target;
    }

private:
    shared_ptr<GameObject> _modelObject;
    unordered_map<EnemyState, int32> _animMap;
    EnemyState _state = EnemyState::Idle;
    shared_ptr<Character> _target;
};

