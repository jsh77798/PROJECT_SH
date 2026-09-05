#include "pch.h"
#include "PlayerController.h"
#include "Transform.h"

void PlayerController::Awake()
{
	
}

void PlayerController::Update()
{
    printf("PlayerController Update\n");

    Vec3 pos = _player->GetTransform()->GetPosition();
    auto movement = _player->GetCharacterMovement();

    bool isMoving = false;

    if (INPUT->GetButton(KEY_TYPE::W)) {
        movement->AddMovementInput(GetTransform()->GetForward());
        isMoving = true;
    }

    if (INPUT->GetButton(KEY_TYPE::S)) {
        movement->AddMovementInput(-GetTransform()->GetForward());
        isMoving = true;
    }

    if (INPUT->GetButton(KEY_TYPE::A))
    {
        Vec3 rot = GetTransform()->GetLocalRotation();
        rot.y -= _rotSpeed * DT;
        GetTransform()->SetLocalRotation(rot);
    }

    if (INPUT->GetButton(KEY_TYPE::D))
    {
        Vec3 rot = GetTransform()->GetLocalRotation();
        rot.y += _rotSpeed * DT;
        GetTransform()->SetLocalRotation(rot);
    }

    if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
    {
		_player->Attack();
    }

    // =========================
    //  애니메이션 처리
    // =========================
    if (isMoving) 
    {
		_player->Move();
    }
    else 
    {
        _player->Stop();
    }


    // 디버그 출력
    {
        char buffer[256];
        sprintf_s(
            buffer,
            sizeof(buffer),
            "isMoving : %s\n",
            isMoving ? "true" : "false"
        );

        OutputDebugStringA(buffer);
    }
}