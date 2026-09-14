#pragma once
#include "Character.h"
#include "GameObject.h"
#include "ModelAnimation.h"
#include "Weapon.h"

enum class PlayerState : uint8
{
    Idle,
    LeftTurn,
    RightTurn,
    Move,
    BackMove,
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
    void BackMove();
    void Turn(float direction);
    void Stop();
    void Attack();

    void EquipWeapon(shared_ptr<Weapon> weapon);

    shared_ptr<Weapon> GetWeapon()
    {
        return _weapon;
    }

private:
    shared_ptr<GameObject> _modelObject;
    shared_ptr<GameObject> _camera;
    unordered_map<PlayerState, string> _animMap;
    PlayerState _state = PlayerState::Idle;

    shared_ptr<Weapon> _weapon;
    shared_ptr<GameObject> _weaponSocket;
};

