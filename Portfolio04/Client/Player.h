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
    PipeAttack,
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

    bool IsAttacking() const
    {
        return _state == PlayerState::PipeAttack;
    }

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
    float _previousAttackProgress = 0.f;
    // 실제 휘두르는 구간에 맞춰 조절
    float _attackHitStart = 15.f / 48.f;
    float _attackHitEnd = 23.f / 48.f;
};

