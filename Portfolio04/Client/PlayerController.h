#pragma once
#include "MonoBehaviour.h"
#include "Player.h"
#include "ModelAnimator.h"
#include "CharacterMovement.h"

class PlayerController : public MonoBehaviour
{
    using Super = MonoBehaviour;

public:
    virtual void Awake() override;
    virtual void Update() override;

    void SetPlayer(Player* player)
    {
        _player = player;
    }

private:
    Player* _player = nullptr;
	
private:
    float _rotSpeed = 2.5f;
};
