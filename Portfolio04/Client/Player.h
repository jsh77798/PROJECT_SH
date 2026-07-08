#pragma once
#include "GameObject.h"
#include "ModelAnimation.h"

enum class PlayerState : uint8
{
    Idle,
    Move,
    Attack,
};

class Player : public GameObject
{
public:
    Player();
    virtual ~Player();

public:
    void Init();
    void Update();

    void ChangeState(PlayerState state);

	void Move();
    void Stop();

private:
    shared_ptr<GameObject> _playerObject;
    shared_ptr<GameObject> _modelObject;
    shared_ptr<GameObject> _obj;
    shared_ptr<GameObject> _camera;
    unordered_map<PlayerState, int32> _animMap;
    PlayerState _state;
};

