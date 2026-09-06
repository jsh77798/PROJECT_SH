#pragma once
#include "Weapon.h"

class Pipe : public Weapon
{
public:
    Pipe();
    virtual ~Pipe();

public:
    virtual void Init();
    virtual void Update() override;

public:
    virtual void Attack() override;

    float GetDamage() const
    {
        return _damage;
    }

protected:
    float _damage = 10.f;
};

