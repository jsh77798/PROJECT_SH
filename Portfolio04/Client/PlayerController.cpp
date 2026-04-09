#include "pch.h"
#include "PlayerController.h"
#include "Transform.h"

void PlayerController::Update()
{
    printf("PlayerController Update\n");

    float dt = TIME->GetDeltaTime();

    Vec3 pos = GetTransform()->GetPosition();

	// 이동
    if (INPUT->GetButton(KEY_TYPE::W))
        pos += GetTransform()->GetLook() * _moveSpeed * dt;

    if (INPUT->GetButton(KEY_TYPE::S))
        pos -= GetTransform()->GetLook() * _moveSpeed * dt;

    //if (INPUT->GetButton(KEY_TYPE::A))
    //    pos -= GetTransform()->GetRight() * _moveSpeed * dt;
    //
    //if (INPUT->GetButton(KEY_TYPE::D))
    //    pos += GetTransform()->GetRight() * _moveSpeed * dt;

    GetTransform()->SetPosition(pos);

	// 회전
    if (INPUT->GetButton(KEY_TYPE::A))
    {
        Vec3 rotation = GetTransform()->GetLocalRotation();
        rotation.y -= _rotSpeed * dt;
        GetTransform()->SetLocalRotation(rotation);
    }

    if (INPUT->GetButton(KEY_TYPE::D))
    {
        Vec3 rotation = GetTransform()->GetLocalRotation();
        rotation.y += _rotSpeed * dt;
        GetTransform()->SetLocalRotation(rotation);
    }
}