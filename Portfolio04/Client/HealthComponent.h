#pragma once
#include "MonoBehaviour.h"

class HealthComponent : public MonoBehaviour
{
public:
    HealthComponent();

    void TakeDamage(float damage);

    float GetHealth() const
    {
        return _health;
    }

    float GetMaxHealth() const
    {
        return _maxHealth;
    }

    void SetMaxHealth(float maxHealth) 
    {
        _maxHealth = maxHealth;
        _health = _maxHealth;
    }

    bool IsDead() const
    {
        return _health <= 0.f;
    }

private:
    float _maxHealth = 100.f;
    float _health = _maxHealth;
};

