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
#include "SoundManager.h"

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
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Move2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_BackMove.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Attack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_PipeAttack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_Hit.fbx"));
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
	_animMap[PlayerState::Run] = animator->MakeAnimData("Run", model->FindAnimation(L"HarryMason/HarryMason_Move2"));
	_animMap[PlayerState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"HarryMason/HarryMason_Attack1"), false);
	_animMap[PlayerState::PipeAttack] = animator->MakeAnimData("PipeAttack", model->FindAnimation(L"HarryMason/HarryMason_PipeAttack1"), false);
	_animMap[PlayerState::Hit] = animator->MakeAnimData("Hit", model->FindAnimation(L"HarryMason/HarryMason_Hit"), false);

	// Camera
	auto camScript = make_shared<CameraScript>();
	camScript->SetTarget(shared_from_this());
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(camScript);
	_doorFollowCamera = camScript;
	_camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);

	// Weapon
	auto socket = make_shared<WeaponSocket>();
	socket->SetBoneName(L"10");
	socket->SetAnimator(_modelObject->GetModelAnimator());
	_weaponSocket = make_shared<GameObject>();
	_weaponSocket->GetOrAddTransform();
	_weaponSocket->AddComponent(socket);
	_modelObject->AddChild(_weaponSocket); // ★ Player의 자식으로 등록

	// Weapon- Pipe
	auto pipe = make_shared<Pipe>();
	pipe->Init();
	pipe->GetOrAddTransform()->SetPosition(Vec3(200.f, 3000.f, 9000.f));
    pipe->GetOrAddTransform()->SetScale(Vec3(100.f));
	pipe->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
    EquipWeapon(pipe);

	// * Player *
	GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 10.0f, 0.0f });
	GetCharacterMovement()->SetMoveSpeed(5.0f);
	AddComponent(playerController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
	CUR_SCENE->Add(_camera);
}

void Player::Update()
{
	GameObject::Update();

	UpdateFootsteps();

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

void Player::Run()
{
	if (IsAttacking())
		return;

	ChangeState(PlayerState::Run);
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

void Player::Attack()
{
	if (_state == PlayerState::PipeAttack)
		return;

	if (_weapon == nullptr)
		return;

	_previousAttackProgress = 0.f;

	_weapon->BeginAttack();

	ChangeState(PlayerState::PipeAttack);

	// 공격 시작 시 한 번 재생
	SoundManager::Get().PlaySFX("PipeSwing");
}

void Player::EquipWeapon(shared_ptr<Weapon> weapon)
{
	if (weapon == nullptr)
		return;

	_weapon = weapon;
	_weapon->SetOwner(shared_from_this());

	_weaponSocket->AddChild(_weapon);
}

void Player::ResetCameraAfterTeleport()
{
	if (auto cameraScript = _doorFollowCamera.lock())
	{
		cameraScript->ResetFollow();
	}
}

void Player::UpdateFootsteps()
{
	const Vec3 position =
		GetTransform()->GetPosition();

	if (!_footstepPositionInitialized)
	{
		_previousFootstepPosition = position;
		_footstepPositionInitialized = true;
		return;
	}

	// 이전 프레임의 실제 이동량 확인
	Vec3 displacement =
		position - _previousFootstepPosition;

	_previousFootstepPosition = position;
	displacement.y = 0.f;

	auto resetProgress = [&]()
		{
			_footstepProgressInitialized = false;
			_footstepAnimation.clear();
		};

	auto movement = GetCharacterMovement();

	if (!movement ||
		!_modelObject ||
		movement->IsMovementPaused() ||
		!movement->IsGrounded())
	{
		resetProgress();
		return;
	}

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
	{
		resetProgress();
		return;
	}

	string animationName;
	float eventA = 0.f;
	float eventB = 0.f;
	bool running = false;

	switch (_state)
	{
	case PlayerState::Move:
		animationName = "Move";
		eventA = _walkFootstepA;
		eventB = _walkFootstepB;
		break;

	case PlayerState::Run:
		animationName = "Run";
		eventA = _runFootstepA;
		eventB = _runFootstepB;
		running = true;
		break;

	case PlayerState::BackMove:
		animationName = "BackMove";
		eventA = _backFootstepA;
		eventB = _backFootstepB;
		break;

	default:
		resetProgress();
		return;
	}

	float progress = 0.f;

	if (!animator->GetAnimationProgress(
		animationName,
		progress))
	{
		resetProgress();
		return;
	}

	// 걷기 -> 달리기 등 애니메이션 변경 시 기준 재설정
	if (!_footstepProgressInitialized ||
		_footstepAnimation != animationName)
	{
		_footstepAnimation = animationName;
		_previousFootstepProgress = progress;
		_footstepProgressInitialized = true;
		return;
	}

	const float previous = _previousFootstepProgress;
	_previousFootstepProgress = progress;

	// 벽에 막혀 애니메이션만 재생되는 경우 소리 생략
	// 진행률은 위에서 계속 갱신하므로 나중에 몰아서 재생되지 않음
	if (displacement.LengthSquared() < 0.00000001f)
		return;

	auto crossed = [&](float eventTime) -> bool
		{
			if (progress >= previous)
			{
				return previous < eventTime &&
					progress >= eventTime;
			}

			// 반복 애니메이션의 끝 -> 시작 구간
			return previous < eventTime ||
				progress >= eventTime;
		};

	// 한 프레임에 두 이벤트를 지나더라도 소리는 한 번만
	if (crossed(eventA) || crossed(eventB))
	{
		PlayFootstep(running);
	}
}

void Player::PlayFootstep(bool running)
{
	const char* soundName =
		_nextFootstepSound == 0
		? "Footstep01"
		: "Footstep02";

	_nextFootstepSound = 1 - _nextFootstepSound;

	SoundManager::Get().PlaySFX(
		soundName,
		running ? 0.85f : 0.55f
	);
}

