#include "pch.h"
#include "WeaponSocket.h"
#include "ModelAnimator.h"
//#include "Transform.h"

WeaponSocket::WeaponSocket()
{
}

void WeaponSocket::Update()
{
    shared_ptr<ModelAnimator> animator =
        _animator.lock();

    if (animator == nullptr)
        return;

    Matrix boneMatrix =
        animator->GetBoneTransform(_boneName);

    Vec3 scale;
    Quaternion rotation;
    Vec3 position;

    boneMatrix.Decompose(
        scale,
        rotation,
        position);

    GetTransform()->SetLocalPosition(position);

    GetTransform()->SetLocalRotation(
        Transform::ToEulerAngles(rotation));
}
