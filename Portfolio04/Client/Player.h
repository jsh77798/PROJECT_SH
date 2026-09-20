#pragma once
#include "Character.h"
#include "GameObject.h"
#include "ModelAnimation.h"
#include "Weapon.h"

class CameraScript;

enum class PlayerState : uint8
{
    Idle,
    LeftTurn,
    RightTurn,
    Move,
    BackMove,
    Run,
    Attack,
    PipeAttack,
    Dead,
    Hit,
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
    void Run();
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

    void ResetCameraAfterTeleport();

    void UpdateFootsteps();
    void PlayFootstep(bool running);

private:
    shared_ptr<GameObject> _modelObject;
    shared_ptr<GameObject> _camera;
    unordered_map<PlayerState, string> _animMap;
    PlayerState _state = PlayerState::Idle;
    weak_ptr<CameraScript> _doorFollowCamera;

    shared_ptr<Weapon> _weapon;
    shared_ptr<GameObject> _weaponSocket;
    float _previousAttackProgress = 0.f;
    // 실제 휘두르는 구간에 맞춰 조절
    float _attackHitStart = 15.f / 48.f;
    float _attackHitEnd = 23.f / 48.f;

    float _walkSpeed = 5.f;
    float _runSpeed = 8.f;

    Vec3 _previousFootstepPosition = Vec3::Zero;
    bool _footstepPositionInitialized = false;

    string _footstepAnimation;
    float _previousFootstepProgress = 0.f;
    bool _footstepProgressInitialized = false;

    int _nextFootstepSound = 0;

    // 한 애니메이션 주기에서 양발이 닿는 시점
    // 우선 예시 값이며 실제 애니메이션에 맞춰 조절
    float _walkFootstepA = 0.2f;
    float _walkFootstepB = 0.7f;

    float _runFootstepA = 0.2f;
    float _runFootstepB = 0.7f;

    float _backFootstepA = 0.2f;
    float _backFootstepB = 0.7f;
};

