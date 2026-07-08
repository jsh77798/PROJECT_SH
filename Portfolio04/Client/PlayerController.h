#pragma once
#include "MonoBehaviour.h"
#include "Player.h"
#include "ModelAnimator.h"

class PlayerController : public MonoBehaviour
{
public:
    virtual void Awake() override;
    virtual void Update() override;

    void SetPlayer(Player* player)
    {
        _player = player;
    }

private:
    Player* _player = nullptr;
    shared_ptr<ModelAnimator> _animator;

private:
    float _moveSpeed = 2.0f;
    float _rotSpeed = 2.5f;
};
