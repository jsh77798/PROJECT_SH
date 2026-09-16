#include "pch.h"
#include <map>
#include "Player.h"
#include "PlayerController.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Material.h"
#include "Model.h"
#include "ModelAnimator.h"
#include "AssetImporter.h"
#include "SphereCollider.h"
#include "CharacterMovement.h"
#include "HealthComponent.h"
#include "Pipe.h"
#include "WeaponSocket.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Init()
{
	InitCharacter();


	//////////////////////////// ResourceData ////////////////////////////
	
	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"HarryMason/HarryMason.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"HarryMason/HarryMason.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Idle.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_LeftTurn.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_RightTurn.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Move.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_BackMove.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Attack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_PipeAttack1.fbx"));
	//////////////////////////////////////////////////////////////////////


	// Movement
	_movement->SetStepHeight(0.3f);
	_movement->SetGroundSnapDistance(0.15f);
	_movement->SetFootOffset(1.2f);

	// PlayerController
	auto playerController = make_shared<PlayerController>();
	playerController->SetPlayer(this);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.5f);
	collider->SetUseFootPosition(false);

	// Health
	_health->SetMaxHealth(1000.f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.00005f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Animation Data
	auto animator = _modelObject->GetModelAnimator();
	_animMap[PlayerState::Idle] = animator->MakeAnimData("Idle", model->FindAnimation(L"HarryMason/HarryMason_Idle"));
	_animMap[PlayerState::LeftTurn] = animator->MakeAnimData("LeftTurn", model->FindAnimation(L"HarryMason/HarryMason_LeftTurn"));
	_animMap[PlayerState::RightTurn] = animator->MakeAnimData("RightTurn", model->FindAnimation(L"HarryMason/HarryMason_RightTurn"));
	_animMap[PlayerState::Move] = animator->MakeAnimData("Move", model->FindAnimation(L"HarryMason/HarryMason_Move"));
	_animMap[PlayerState::BackMove] = animator->MakeAnimData("BackMove", model->FindAnimation(L"HarryMason/HarryMason_BackMove"));
	_animMap[PlayerState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"HarryMason/HarryMason_Attack1"), false);
	_animMap[PlayerState::PipeAttack] = animator->MakeAnimData("PipeAttack", model->FindAnimation(L"HarryMason/HarryMason_PipeAttack1"), false);

	// Camera
	auto camScript = make_shared<CameraScript>();
	camScript->SetTarget(shared_from_this());
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(camScript);
	_camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);

	// Weapon
	auto socket = make_shared<WeaponSocket>();
	socket->SetBoneName(L"10");
	socket->SetAnimator(_modelObject->GetModelAnimator());
	_weaponSocket = make_shared<GameObject>();
	_weaponSocket->GetOrAddTransform();
	_weaponSocket->AddComponent(socket);
	// ★ Player의 자식으로 등록
	_modelObject->AddChild(_weaponSocket);

	// Pipe
	auto pipe = make_shared<Pipe>();
	pipe->Init();
	pipe->GetOrAddTransform()->SetPosition(Vec3(200.f, 3000.f, 9000.f));
    pipe->GetOrAddTransform()->SetScale(Vec3(100.f));
	pipe->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
    EquipWeapon(pipe);

	// * Player *
	GetOrAddTransform()->SetPosition(Vec3{ 40.0f, 10.0f, 100.0f });
	GetCharacterMovement()->SetMoveSpeed(5.0f);
	AddComponent(playerController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
	CUR_SCENE->Add(_camera);
}

void Player::Update()
{
	GameObject::Update();

	if (_state != PlayerState::PipeAttack)
		return;

	if (_modelObject == nullptr)
		return;

	auto animator = _modelObject->GetModelAnimator();
	if (animator == nullptr)
		return;

	float progress = 0.f;

	if (animator->GetAnimationProgress("PipeAttack", progress))
	{
		// 이전~현재 진행 구간이 타격 구간과 겹치는지 검사
		// 한 프레임에 타격 구간을 넘어가도 한 번은 검사
		bool crossedHitWindow =
			progress >= _previousAttackProgress &&
			progress >= _attackHitStart &&
			_previousAttackProgress <= _attackHitEnd;

		if (crossedHitWindow && _weapon)
		{
			_weapon->Attack();
		}

		_previousAttackProgress = progress;
	}

	if (animator->IsAnimationFinished())
	{
		if (_weapon)
			_weapon->EndAttack();

		ChangeState(PlayerState::Idle);
	}
}

void Player::ChangeState(PlayerState state)
{
	if (_state == state)
		return;

	_state = state;

	if (_modelObject)
	{
		auto animator =
			_modelObject->GetModelAnimator();

		if (animator)
		{
			//animator->SetLoop(
			//	state != PlayerState::Dead);

			animator->Play(_animMap[state]);
		}
	}
}

void Player::Move()
{
	if (_state == PlayerState::PipeAttack)
		return;

	ChangeState(PlayerState::Move);
}

void Player::BackMove()
{
	if (_state == PlayerState::PipeAttack)
		return;

	ChangeState(PlayerState::BackMove);
}

void Player::Turn(float direction)
{
	if (_state == PlayerState::PipeAttack)
		return;

	if (direction == 0.f)
	{
		Stop();
		return;
	}

	ChangeState(
		direction < 0.f
		? PlayerState::LeftTurn
		: PlayerState::RightTurn
	);
}

void Player::Stop()
{
	if (_state == PlayerState::PipeAttack)
		return;

	ChangeState(PlayerState::Idle);
}

//이후 무기 시스템을 만드면 Attack시 무리와 연동하여 무기에서 데미지를 입히도록 변경해야함
void Player::Attack()
{
	if (_state == PlayerState::PipeAttack)
		return;

	if (_weapon == nullptr)
		return;

	_previousAttackProgress = 0.f;

	_weapon->BeginAttack();

	ChangeState(PlayerState::PipeAttack);
}

void Player::EquipWeapon(shared_ptr<Weapon> weapon)
{
	if (weapon == nullptr)
		return;

	_weapon = weapon;
	_weapon->SetOwner(shared_from_this());

	_weaponSocket->AddChild(_weapon);
}

