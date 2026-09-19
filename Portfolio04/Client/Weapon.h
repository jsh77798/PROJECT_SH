#pragma once
#include "GameObject.h"
#include <unordered_set>

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

    void SetOwner(shared_ptr<GameObject> owner)
    {
        _owner = owner;
    }

    void BeginAttack()
    {
        _hitObjects.clear();
        _attackActive = true;
    }

    void EndAttack()
    {
        _attackActive = false;
        _hitObjects.clear();
    }

    void SetDamage(float damage)
    {
        _damage = damage;
    }

    float GetDamage() const
    {
        return _damage;
    }

protected:
    float _damage = 90.f;
    bool _attackActive = false;
    weak_ptr<GameObject> _owner;
    std::unordered_set<shared_ptr<GameObject>> _hitObjects;  // 공격 한 번 동안 이미 맞은 대상 보관
};

