#pragma once
#include "Weapon.h"

class Pipe : public Weapon
{
public:
    Pipe();
    virtual ~Pipe();

public:
    virtual void Init() override;
    virtual void Update() override;

public:
    virtual void Attack() override;

    void SetHitRadius(float radius)
    {
        _hitRadius = radius;
    }

private:
    float _hitRadius = 0.3f; // 월드 단위, 실제 파이프 크기에 맞춰 조절
};

