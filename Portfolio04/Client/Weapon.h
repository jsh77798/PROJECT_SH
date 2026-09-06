#pragma once
#include "GameObject.h"

class Weapon : public GameObject
{
public:
    Weapon();
    virtual ~Weapon();

public:
    virtual void Init();
    virtual void Update() override;

public:
    virtual void Attack() = 0;

    void SetDamage(float damage)
    {
        _damage = damage;
    }

    float GetDamage() const
    {
        return _damage;
    }

protected:
    float _damage = 10.f;
};

