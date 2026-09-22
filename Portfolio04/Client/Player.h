#pragma once
#include "Character.h"
#include "GameObject.h"
#include "ModelAnimation.h"
#include "Weapon.h"

class CameraScript;
class Enemy;

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
    KickAttack,
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
    void TryPickupKey();
    void Hit();

    bool IsAttacking() const
    {
        //return _state == PlayerState::PipeAttack;
        return _state == PlayerState::PipeAttack ||
            _state == PlayerState::KickAttack;
    }

    void EquipWeapon(shared_ptr<Weapon> weapon);

    shared_ptr<Weapon> GetWeapon()
    {
        return _weapon;
    }

    void AddKey(const std::string& keyID)
    {
        _keys.insert(keyID);
    }

    bool HasKey(const std::string& keyID) const
    {
        return _keys.find(keyID) != _keys.end();
    }

    bool IsHit() const
    {
        return _state == PlayerState::Hit;
    }

    bool IsActionLocked() const
    {
        return IsAttacking() ||
            _state == PlayerState::Hit ||
            _state == PlayerState::Dead;
    }

    void ResetCameraAfterTeleport();

    void UpdateFootsteps();
    void PlayFootstep(bool running);

private:
    shared_ptr<Enemy> FindKickTarget();
    bool CanKickTarget(const shared_ptr<Enemy>& enemy);
    void ApplyKickDamage();

    struct PipeComboData
    {
        string animation;

        float hitStart;
        float hitEnd;
        float soundProgress;
    };

    // 진행률 = (이벤트 프레임 - 1) / (전체 프레임 - 1)
// 앞서 만든 FBX 3개의 타이밍
    std::array<PipeComboData, 3> _pipeCombos
    { {
        { "", 16.f / 45.f, 22.f / 45.f, 7.f / 45.f },
        { "", 12.f / 42.f, 20.f / 42.f, 4.f / 42.f },
        { "", 22.f / 57.f, 29.f / 57.f, 14.f / 57.f }
    } };

    int _comboIndex = -1;

    // 현재 공격이 끝나면 다음 공격을 실행할지
    bool _comboQueued = false;

    // 현재 공격에서 휘두르는 소리를 재생했는지
    bool _attackSoundPlayed = false;

    // 애니메이션 진행률 15~90% 구간에서 추가 입력 허용
    float _comboInputStart = 0.15f;
    float _comboInputEnd = 0.90f;

    bool BeginPipeCombo(int index);
    void QueueNextCombo();
    void UpdateAttack();
    void ResetAttack();

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
    float _attackHitStart = 10.f / 48.f;
    float _attackHitEnd = 23.f / 48.f;

    float _pipeAttackSpeed = 1.5f;

private:
    weak_ptr<Enemy> _kickTarget;

    bool _kickHitDone = false;

    float _kickRange = 1.5f;
    float _kickHeightTolerance = 0.8f;
    float _kickHalfAngle = 60.f;
    float _kickDamage = 100.f;

    // 애니메이션 전체 길이에 대한 비율
    // 실제 발이 적에게 닿는 구간에 맞춰 조절
    float _kickHitStart = 0.35f;
    float _kickHitEnd = 0.55f;



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

    std::unordered_set<std::string> _keys;
};

