#pragma once
#include "MonoBehaviour.h"
#include <functional>

class HealthComponent : public MonoBehaviour
{
public:
    HealthComponent();

    void TakeDamage(float damage);

    void TakeDamage(
        float damage,
        const Vec3& attackerPosition
    );

    using HitCallback =
        std::function<void(const Vec3&, bool)>;

    void SetOnHit(HitCallback callback)
    {
        _onHit = callback;
    }

    void SetOnDeath(std::function<void()> callback)
    {
        _onDeath = callback;
    }

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
    void ApplyDamage(
        float damage,
        const Vec3& attackerPosition,
        bool hasAttackerPosition
    );

private:
    float _maxHealth = 100.f;
    float _health = _maxHealth;

    HitCallback _onHit;
    std::function<void()> _onDeath;
};

