#pragma once
#include "Character.h"
#include "GameObject.h"
#include "ModelAnimation.h"

enum class PlayerState : uint8
{
    Idle,
    Move,
    Attack,
    Dead,
};

class Player : public Character
{
public:
    Player();
    virtual ~Player();

public:
    virtual void Init() override;
    virtual void Update() override;

    void ChangeState(PlayerState state);

	void Move();
    void Stop();
    void Attack();

private:
    shared_ptr<GameObject> _modelObject;
    shared_ptr<GameObject> _camera;
    unordered_map<PlayerState, string> _animMap;
    PlayerState _state = PlayerState::Idle;
};

