#include "pch.h"
#include "HealthComponent.h"

HealthComponent::HealthComponent()
{
}

void HealthComponent::TakeDamage(float damage)
{
    if (damage <= 0.f)
        return;

    if (IsDead())
        return;

    _health -= damage;

    if (_health < 0.f)
        _health = 0.f;

    cout << "Health : "
        << _health
        << " / "
        << _maxHealth
        << endl;
}