#pragma once
#include "MonoBehaviour.h"

class WeaponSocket : public MonoBehaviour
{
    using Super = MonoBehaviour;

public:
    WeaponSocket();

    void SetBoneName(const wstring& boneName)
    {
        _boneName = boneName;
    }

    void SetAnimator(
        shared_ptr<ModelAnimator> animator)
    {
        _animator = animator;
    }

    virtual void Update() override;

private:
    wstring _boneName;

    weak_ptr<ModelAnimator> _animator;
};

