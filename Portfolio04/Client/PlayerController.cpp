#include "pch.h"
#include "PlayerController.h"
#include "Transform.h"

void PlayerController::Awake()
{
	//_player = static_pointer_cast<Player>(GetGameObject());
    _animator = GetGameObject()->GetModelAnimator();
}

void PlayerController::Update()
{
    printf("PlayerController Update\n");

    float dt = TIME->GetDeltaTime();

    Vec3 pos = GetTransform()->GetPosition();

    bool isMoving = false;

	// 이동
    if (INPUT->GetButton(KEY_TYPE::W)) 
    {
        pos += GetTransform()->GetForward() * _moveSpeed * dt;
        isMoving = true;
    }
     
    if (INPUT->GetButton(KEY_TYPE::S)) {
        pos -= GetTransform()->GetForward() * _moveSpeed * dt;
		isMoving = true;
    }

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
}