#pragma once
#include "Component.h"

class CharacterMovement : public Component
{
    using Super = Component;
public:
    CharacterMovement();
    ~CharacterMovement();

    virtual void Update() override;
    void LateUpdate() override;

    bool IsGrounded() const { return _isGrounded; }

    void SetStepHeight(float height)
    {
        _stepHeight = height;
    }

    void SetGroundSnapDistance(float distance)
    {
        _groundSnapDistance = distance;
    }

    void SetFootOffset(float offset)
    {
        _footOffset = offset;
    }

    void AddMovementInput(const Vec3& direction);

    void ClearMovementInput()
    {
        _inputVector = Vec3::Zero;
    }

    void SetMoveSpeed(float speed) { _moveSpeed = speed; };

    void SetRotationSpeed(float speed) { _rotationSpeed = speed; };

    void Move(const Vec3& delta);
    void RotateTo(const Vec3& direction);

private:
    void TickMovement();
    void TickVertical(float dt);
    bool ResolveCollision(Vec3& normal);
    void Slide(Vec3& position);
    void SetPositionAndSync(const Vec3& position);
    bool FindGround(
        const Vec3& position,
        float referenceFootY,
        float& groundY
    );
    bool FindGroundInRange(
        const Vec3& position,
        float minGroundY,
        float maxGroundY,
        float& groundY
    );
    bool HasBlockingCollision();
    bool TryGroundMove(const Vec3& movement);

private:
    Vec3 _inputVector = Vec3::Zero;
    float _moveSpeed = 0.f;
	float _rotationSpeed = XMConvertToRadians(180.0f);
    float _stepHeight = 0.3f;
    float _groundSnapDistance = 0.4f;
    float _groundOffset = 0.002f;
    float _footOffset = 0.8f;
    bool _isGrounded = false;

    float _verticalVelocity = 0.f;
    float _gravity = 9.8f;
    float _maxFallSpeed = 30.f;
};

