#include "pch.h"
#include "PlayerController.h"
#include "Transform.h"

void PlayerController::Awake()
{
	
}

void PlayerController::Update()
{
    //printf("PlayerController Update\n");

    if (!_player)
        return;

    auto movement = _player->GetCharacterMovement();

    if (!movement)
        return;

    // 문 전환 중 이동·회전·공격 입력 차단
    if (movement->IsMovementPaused())
    {
        movement->ClearMovementInput();
        return;
    }

    if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
    {
        _player->Attack();
    }

    if (_player->IsActionLocked())
    {
        movement->ClearMovementInput();
        return;
    }

    if (_player->IsAttacking())
    {
        movement->ClearMovementInput();
        return;
    }

    if (INPUT->GetButtonDown(KEY_TYPE::E))
    {
        _player->TryPickupKey();
    }

    float moveInput = 0.f;
    float turnInput = 0.f;

    if (INPUT->GetButton(KEY_TYPE::W)) {
        moveInput += 1.f;
    }

    if (INPUT->GetButton(KEY_TYPE::S)) {
        moveInput -= 1.f;
    }

    if (INPUT->GetButton(KEY_TYPE::A))
        turnInput -= 1.f;

    if (INPUT->GetButton(KEY_TYPE::D))
        turnInput += 1.f;

    // 앞으로 이동하면서 Shift를 누를 때만 달리기
    const bool isRunning =
        moveInput > 0.f &&
        INPUT->GetButton(KEY_TYPE::SHIFT);

    movement->SetMoveSpeed(
        isRunning ? _runSpeed : _walkSpeed
    );

    //auto transform = _player->GetTransform();

    if (turnInput != 0.f)
    {
        Vec3 rot = GetTransform()->GetLocalRotation();
        rot.y += turnInput * _rotSpeed * DT;
        GetTransform()->SetLocalRotation(rot);
    }

    // 이동
    if (moveInput != 0.f)
    {
        Vec3 forward = GetTransform()->GetForward();

        movement->AddMovementInput(
            forward * moveInput
        );
    }

    // =========================
    //  애니메이션 처리
    // =========================
    if (moveInput > 0.f)
    {
        if (isRunning)
            _player->Run();
        else
            _player->Move();
    }
    else if (moveInput < 0.f)
    {
        _player->BackMove();
    }
    else if (turnInput != 0.f)
    {
        _player->Turn(turnInput);
    }
    else 
    {
        _player->Stop();
    }

}