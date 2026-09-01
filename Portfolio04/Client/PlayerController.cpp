#include "pch.h"
#include "PlayerController.h"
#include "Transform.h"

void PlayerController::Awake()
{
	//_player = static_pointer_cast<Player>(GetGameObject());
    _movement = static_pointer_cast<CharacterMovement>(GetGameObject()->GetFixedComponent(ComponentType::CharacterMovement));
    _animator = GetGameObject()->GetModelAnimator();
}

void PlayerController::Update()
{
    printf("PlayerController Update\n");

    Vec3 pos = _player->GetTransform()->GetPosition();

    bool isMoving = false;

    if (INPUT->GetButton(KEY_TYPE::W)) {
        _movement->AddMovementInput(GetTransform()->GetForward());
        isMoving = true;
    }

    if (INPUT->GetButton(KEY_TYPE::S)) {
        _movement->AddMovementInput(-GetTransform()->GetForward());
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

    // =========================
    //  局聪皋捞记 贸府
    // =========================
    if (isMoving) 
    {
		_player->Move();
    }
    else 
    {
        _player->Stop();
    }
}