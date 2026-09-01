#pragma once
#include "Component.h"

class CharacterMovement : public Component
{
    using Super = Component;
public:
    CharacterMovement();
    ~CharacterMovement();

    virtual void Update() override;

    void AddMovementInput(const Vec3& direction);

    void SetMoveSpeed(float speed) { _moveSpeed = speed; };

    void SetRotationSpeed(float speed) { _rotationSpeed = speed; };

    void Move(const Vec3& delta);
    void RotateTo(const Vec3& direction);

private:
    void TickMovement();
    bool ResolveCollision();
    void Slide(Vec3& position);

private:
    Vec3 _inputVector = Vec3::Zero;
    float _moveSpeed = 0.f;
	float _rotationSpeed = XMConvertToRadians(180.0f);
};

