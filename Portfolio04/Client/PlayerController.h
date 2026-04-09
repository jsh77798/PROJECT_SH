#pragma once
#include "MonoBehaviour.h"

class PlayerController : public MonoBehaviour
{
public:
    virtual void Update() override;

private:
    float _moveSpeed = 5.0f;
    float _rotSpeed = 2.5f;
};
