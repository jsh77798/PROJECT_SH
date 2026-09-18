#include "pch.h"
#include "HealthComponent.h"

HealthComponent::HealthComponent()
{
}

void HealthComponent::TakeDamage(float damage)
{
    ApplyDamage(damage, Vec3::Zero, false);
}

void HealthComponent::TakeDamage(float damage, const Vec3& attackerPosition)
{
    ApplyDamage(damage, attackerPosition, true);
}

void HealthComponent::ApplyDamage(float damage, const Vec3& attackerPosition, bool hasAttackerPosition)
{
    if (damage <= 0.f || IsDead())
        return;

    _health -= damage;

    if (_health < 0.f)
        _health = 0.f;

    cout << "Health : "
        << _health
        << " / "
        << _maxHealth
        << endl;

    if (IsDead())
    {
        // 콜백 안에서 등록 상태가 바뀌어도 안전하게 복사
        auto callback = _onDeath;

        if (callback)
            callback();

        return;
    }

    auto callback = _onHit;

    if (callback)
    {
        callback(
            attackerPosition,
            hasAttackerPosition
        );
    }
}
