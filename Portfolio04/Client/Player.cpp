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
#include "KeyItem.h"
#include "Enemy.h"
#include "Scene.h"
#include "BaseCollider.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Init()
{
	InitCharacter();

	weak_ptr<Player> weakSelf =
		static_pointer_cast<Player>(shared_from_this());

	_health->SetOnHit(
		[weakSelf](
			const Vec3& attackerPosition,
			bool hasAttackerPosition)
		{
			auto player = weakSelf.lock();

			if (!player)
				return;

			// 현재 플레이어는 공통 피격 애니메이션 사용
			player->Hit();
		}
	);

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
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_PipeCombo01.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_PipeCombo02.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"HarryMason/HarryMason_PipeCombo03.fbx"));
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
	_animMap[PlayerState::KickAttack] = animator->MakeAnimData("KickAttack", model->FindAnimation(L"HarryMason/HarryMason_Attack1"), false);
	_pipeCombos[0].animation = animator->MakeAnimData("PipeCombo01",model->FindAnimation(L"HarryMason/HarryMason_PipeCombo01"),false);
	_pipeCombos[1].animation = animator->MakeAnimData("PipeCombo02",model->FindAnimation(L"HarryMason/HarryMason_PipeCombo02"),false);
	_pipeCombos[2].animation = animator->MakeAnimData("PipeCombo03",model->FindAnimation(L"HarryMason/HarryMason_PipeCombo03"),false);
	_animMap[PlayerState::PipeAttack] = _pipeCombos[0].animation;
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
	//GameObject::Update();
	//
	//UpdateFootsteps();
	//
	//// 피격 상태 처리
	//if (_state == PlayerState::Hit)
	//{
	//	if (auto movement = GetCharacterMovement())
	//		movement->ClearMovementInput();
	//
	//	if (!_modelObject)
	//		return;
	//
	//	auto animator = _modelObject->GetModelAnimator();
	//
	//	if (!animator)
	//		return;
	//
	//	float hitProgress = 0.f;
	//
	//	// Hit 애니메이션의 진행률까지 확인한 뒤 종료
	//	if (animator->GetAnimationProgress("Hit", hitProgress))
	//	{
	//		if (hitProgress >= 0.99f &&
	//			animator->IsAnimationFinished())
	//		{
	//			ChangeState(PlayerState::Idle);
	//		}
	//	}
	//
	//	return;
	//}
	//
	//if (!IsAttacking())
	//	return;
	//
	//if (!_modelObject)
	//	return;
	//
	//auto animator = _modelObject->GetModelAnimator();
	//
	//if (!animator)
	//	return;
	//
	//const PlayerState attackState = _state;
	//
	//auto it = _animMap.find(attackState);
	//
	//if (it == _animMap.end())
	//	return;
	//
	//const bool isKick =
	//	attackState == PlayerState::KickAttack;
	//
	//const float hitStart =
	//	isKick ? _kickHitStart : _attackHitStart;
	//
	//const float hitEnd =
	//	isKick ? _kickHitEnd : _attackHitEnd;
	//
	//float progress = 0.f;
	//
	//if (animator->GetAnimationProgress(it->second, progress))
	//{
	//	const bool crossedHitWindow =
	//		progress >= _previousAttackProgress &&
	//		progress >= hitStart &&
	//		_previousAttackProgress <= hitEnd;
	//
	//	// 데미지 콜백 실행 전에 진행률 저장
	//	_previousAttackProgress = progress;
	//
	//	if (crossedHitWindow)
	//	{
	//		if (isKick)
	//		{
	//			ApplyKickDamage();
	//		}
	//		else if (_weapon)
	//		{
	//			_weapon->Attack();
	//		}
	//	}
	//}
	//
	//// 데미지 처리 과정에서 상태가 변경되었다면 덮어쓰지 않음
	//if (_state != attackState)
	//	return;
	//
	//if (animator->IsAnimationFinished())
	//{
	//	if (_weapon)
	//		_weapon->EndAttack();
	//
	//	_kickTarget.reset();
	//	_kickHitDone = false;
	//
	//	ChangeState(PlayerState::Idle);
	//}

	GameObject::Update();

	UpdateFootsteps();

	if (_state == PlayerState::Hit)
	{
		if (auto movement = GetCharacterMovement())
			movement->ClearMovementInput();

		if (!_modelObject)
			return;

		auto animator = _modelObject->GetModelAnimator();

		if (!animator)
			return;

		float hitProgress = 0.f;

		if (animator->GetAnimationProgress(
			"Hit",
			hitProgress))
		{
			if (hitProgress >= 0.99f &&
				animator->IsAnimationFinished())
			{
				ChangeState(PlayerState::Idle);
			}
		}

		return;
	}

	UpdateAttack();
}

void Player::ChangeState(PlayerState state)
{
	//if (_state == state)
	//	return;
	//
	//_state = state;
	//
	//if (_modelObject)
	//{
	//	auto animator =
	//		_modelObject->GetModelAnimator();
	//
	//	if (animator)
	//	{
	//		//animator->SetLoop(
	//		//	state != PlayerState::Dead);
	//
	//		animator->Play(_animMap[state]);
	//	}
	//}
	if (_state == state)
		return;

	if (IsAttacking())
		ResetAttack();

	_state = state;

	if (!_modelObject)
		return;

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
		return;

	auto it = _animMap.find(state);

	if (it == _animMap.end() || it->second.empty())
		return;

	animator->Play(it->second);
}

void Player::Move()
{
	if (IsActionLocked())
		return;

	ChangeState(PlayerState::Move);
}

void Player::BackMove()
{
	if (IsActionLocked())
		return;

	ChangeState(PlayerState::BackMove);
}

void Player::Run()
{
	if (IsActionLocked())
		return;

	ChangeState(PlayerState::Run);
}

void Player::Turn(float direction)
{
	if (IsActionLocked())
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
	if (IsActionLocked())
		return;

	ChangeState(PlayerState::Idle);
}

void Player::Attack()
{
	//if (IsActionLocked())
	//	return;
	//
	//auto movement = GetCharacterMovement();
	//auto health = GetHealthComponent();
	//
	//if (!movement || movement->IsMovementPaused())
	//	return;
	//
	//if (!health || health->IsDead())
	//	return;
	//
	//if (!_modelObject || !_modelObject->GetModelAnimator())
	//	return;
	//
	//auto target = FindKickTarget();
	//
	//if (target)
	//{
	//	auto it = _animMap.find(PlayerState::KickAttack);
	//
	//	if (it != _animMap.end() && !it->second.empty())
	//	{
	//		_kickTarget = target;
	//		_kickHitDone = false;
	//		_previousAttackProgress = 0.f;
	//
	//		movement->ClearMovementInput();
	//
	//		// 파이프 판정이 남지 않도록 종료
	//		if (_weapon)
	//			_weapon->EndAttack();
	//
	//		ChangeState(PlayerState::KickAttack);
	//		return;
	//	}
	//}
	//
	//// 기존 파이프 공격
	//if (!_weapon)
	//	return;
	//
	//_kickTarget.reset();
	//_kickHitDone = false;
	//_previousAttackProgress = 0.f;
	//
	//movement->ClearMovementInput();
	//
	//_weapon->BeginAttack();
	//ChangeState(PlayerState::PipeAttack);
	//
	//SoundManager::Get().PlaySFX("PipeSwing");

	auto movement = GetCharacterMovement();
	auto health = GetHealthComponent();

	if (!movement || movement->IsMovementPaused())
		return;

	if (!health || health->IsDead())
		return;

	// 공격 중 추가 클릭은 다음 공격 예약으로 처리
	if (_state == PlayerState::PipeAttack)
	{
		QueueNextCombo();
		return;
	}

	// 발차기·피격·사망 상태에서는 새 공격 금지
	if (IsActionLocked())
		return;

	if (!_modelObject ||
		!_modelObject->GetModelAnimator())
	{
		return;
	}

	// 첫 공격을 시작할 때만 발차기 대상 검사
	auto target = FindKickTarget();

	if (target)
	{
		auto it = _animMap.find(PlayerState::KickAttack);

		if (it != _animMap.end() && !it->second.empty())
		{
			ResetAttack();

			_kickTarget = target;

			movement->ClearMovementInput();

			ChangeState(PlayerState::KickAttack);
			return;
		}
	}

	// 발차기 대상이 없으면 파이프 1타 시작
	BeginPipeCombo(0);
}

void Player::TryPickupKey()
{
	auto movement = GetCharacterMovement();
	auto health = GetHealthComponent();

	if (!movement || !health || health->IsDead())
		return;

	if (IsActionLocked() || movement->IsMovementPaused())
		return;

	shared_ptr<KeyItem> nearestItem;
	float nearestDistanceSquared = FLT_MAX;

	const Vec3 footPosition =
		movement->GetFootPosition();

	// 여기에서는 후보만 찾음
	for (const auto& object : CUR_SCENE->GetObjects())
	{
		auto item = dynamic_pointer_cast<KeyItem>(object);

		if (!item || !item->CanPickup(*this))
			continue;

		Vec3 difference =
			item->GetTransform()->GetPosition() -
			footPosition;

		const float distanceSquared =
			difference.LengthSquared();

		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestItem = item;
		}
	}

	// 씬 순회가 끝난 뒤 획득 및 제거
	if (nearestItem)
	{
		nearestItem->TryPickup(*this);
	}
}

void Player::Hit()
{
	auto health = GetHealthComponent();

	if (!health || health->IsDead())
		return;

	if (_state == PlayerState::Dead)
		return;

	if (!_modelObject)
		return;

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
		return;

	auto it = _animMap.find(PlayerState::Hit);

	if (it == _animMap.end() || it->second.empty())
		return;

	// 진행 중인 공격 중단
	//if (_weapon)
	//	_weapon->EndAttack();
	//
	//_kickTarget.reset();
	//_kickHitDone = false;
	//_previousAttackProgress = 0.f;
	ResetAttack();

	if (auto movement = GetCharacterMovement())
		movement->ClearMovementInput();

	// 이미 피격 중이라면 현재 피격 동작 유지
	if (_state == PlayerState::Hit)
		return;

	ChangeState(PlayerState::Hit);

	SoundManager::Get().PlaySFX("PlayerHit");

	// 기본 생성 위치: 플레이어 루트
	Vec3 bloodPosition = GetTransform()->GetPosition();

	// 실제 몸 콜라이더 중심에 맞춰 생성
	auto sphere = dynamic_pointer_cast<SphereCollider>(
		GetCollider()
	);

	if (sphere)
	{
		const auto& center = sphere->GetBoundingSphere().Center;

		bloodPosition = Vec3(
			center.x,
			center.y,
			center.z
		);
	}

	// 우선 플레이어 뒤쪽으로 피가 튀도록 설정
	Vec3 bloodDirection = -GetTransform()->GetForward();
	bloodDirection.y = 0.f;

	if (bloodDirection.LengthSquared() > 0.000001f)
		bloodDirection.Normalize();
	else
		bloodDirection = Vec3(0.f, 1.f, 1.f);

	CUR_SCENE->SpawnBlood(
		bloodPosition,
		bloodDirection);
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

shared_ptr<Enemy> Player::FindKickTarget()
{
	shared_ptr<Enemy> nearestEnemy;
	float nearestDistanceSquared = FLT_MAX;

	auto movement = GetCharacterMovement();

	if (!movement)
		return nullptr;

	const Vec3 playerFoot = movement->GetFootPosition();

	for (const auto& object : CUR_SCENE->GetObjects())
	{
		auto enemy = dynamic_pointer_cast<Enemy>(object);

		if (!CanKickTarget(enemy))
			continue;

		Vec3 difference =
			enemy->GetCharacterMovement()->GetFootPosition()
			- playerFoot;

		difference.y = 0.f;

		const float distanceSquared = difference.LengthSquared();

		if (distanceSquared < nearestDistanceSquared)
		{
			nearestDistanceSquared = distanceSquared;
			nearestEnemy = enemy;
		}
	}

	return nearestEnemy;
}

bool Player::CanKickTarget(const shared_ptr<Enemy>& enemy)
{
	if (!enemy || !enemy->CanBeKicked())
		return false;

	auto health = enemy->GetHealthComponent();

	if (!health || health->IsDead())
		return false;

	auto movement = GetCharacterMovement();
	auto enemyMovement = enemy->GetCharacterMovement();

	if (!movement || !enemyMovement)
		return false;

	// 루트 중심 높이가 서로 달라도 판단할 수 있도록 발 위치 사용
	const Vec3 playerFoot = movement->GetFootPosition();
	const Vec3 enemyFoot = enemyMovement->GetFootPosition();

	Vec3 direction = enemyFoot - playerFoot;

	if (std::fabs(direction.y) > _kickHeightTolerance)
		return false;

	direction.y = 0.f;

	const float distanceSquared = direction.LengthSquared();

	if (distanceSquared > _kickRange * _kickRange)
		return false;

	if (distanceSquared > 0.000001f)
	{
		direction.Normalize();

		Vec3 forward = GetTransform()->GetForward();
		forward.y = 0.f;

		if (forward.LengthSquared() < 0.000001f)
			return false;

		forward.Normalize();

		const float minDot =
			std::cos(XMConvertToRadians(_kickHalfAngle));

		if (forward.Dot(direction) < minDot)
			return false;
	}

	// 발보다 조금 위에서 벽 검사
	const Vec3 rayStart = playerFoot + Vec3(0.f, 0.35f, 0.f);
	const Vec3 rayEnd = enemyFoot + Vec3(0.f, 0.35f, 0.f);

	Vec3 rayDirection = rayEnd - rayStart;
	const float rayLength = rayDirection.Length();

	if (rayLength < 0.0001f)
		return true;

	rayDirection /= rayLength;

	Ray ray;
	ray.position = rayStart;
	ray.direction = rayDirection;

	shared_ptr<BaseCollider> hitCollider;
	float hitDistance = 0.f;

	// 플레이어와 발차기 대상은 제외하고 장애물만 검사
	bool blocked = CUR_SCENE->RayCastFiltered(
		ray,
		rayLength,
		[this, enemy](const shared_ptr<GameObject>& object)
		{
			return object.get() == this ||
				object == enemy;
		},
		hitCollider,
		hitDistance
	);

	return !blocked;
}

void Player::ApplyKickDamage()
{
	if (_kickHitDone)
		return;

	auto enemy = _kickTarget.lock();

	// 공격 시작 이후 적이 일어나거나 멀어졌다면 명중하지 않음
	if (!CanKickTarget(enemy))
		return;

	auto health = enemy->GetHealthComponent();

	if (!health || health->IsDead())
		return;

	// TakeDamage의 피격 콜백 실행 전에 기록
	_kickHitDone = true;

	health->TakeDamage(
		_kickDamage,
		GetTransform()->GetPosition()
	);

	SoundManager::Get().PlaySFX("PipeHit");
}

bool Player::BeginPipeCombo(int index)
{
	if (index < 0 ||
		index >= static_cast<int>(_pipeCombos.size()))
	{
		return false;
	}

	auto movement = GetCharacterMovement();
	auto health = GetHealthComponent();

	if (!movement || movement->IsMovementPaused())
		return false;

	if (!health || health->IsDead())
		return false;

	if (!_weapon || !_modelObject)
		return false;

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
		return false;

	const auto& combo = _pipeCombos[index];

	if (combo.animation.empty())
		return false;

	// 이전 타격 기록과 예약을 초기화
	ResetAttack();

	_comboIndex = index;

	movement->ClearMovementInput();

	// 각 타격마다 별도의 공격으로 시작
	_weapon->BeginAttack();

	// 1~3타 모두 같은 상태를 사용하므로 직접 재생
	// ChangeState()는 같은 상태라면 return하기 때문
	_state = PlayerState::PipeAttack;
	animator->Play(
		combo.animation,
		_pipeAttackSpeed
	);

	return true;
}

void Player::QueueNextCombo()
{
	if (_state != PlayerState::PipeAttack)
		return;

	// 3타에서는 추가 공격을 예약하지 않음
	if (_comboIndex < 0 ||
		_comboIndex >=
		static_cast<int>(_pipeCombos.size()) - 1)
	{
		return;
	}

	if (_comboQueued || !_modelObject)
		return;

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
		return;

	float progress = 0.f;

	if (!animator->GetAnimationProgress(
		_pipeCombos[_comboIndex].animation,
		progress))
	{
		return;
	}

	if (progress < _comboInputStart ||
		progress > _comboInputEnd)
	{
		return;
	}

	// 여러 번 클릭해도 다음 1타만 예약
	_comboQueued = true;
}

void Player::UpdateAttack()
{
	if (!IsAttacking())
		return;

	auto health = GetHealthComponent();

	if (!health || health->IsDead())
	{
		ResetAttack();
		return;
	}

	if (auto movement = GetCharacterMovement())
		movement->ClearMovementInput();

	if (!_modelObject)
		return;

	auto animator = _modelObject->GetModelAnimator();

	if (!animator)
		return;

	const PlayerState attackState = _state;
	const int attackIndex = _comboIndex;

	const bool isKick =
		attackState == PlayerState::KickAttack;

	string animationName;
	float hitStart = 0.f;
	float hitEnd = 0.f;
	float soundProgress = 0.f;

	if (isKick)
	{
		auto it = _animMap.find(PlayerState::KickAttack);

		if (it == _animMap.end() || it->second.empty())
			return;

		animationName = it->second;
		hitStart = _kickHitStart;
		hitEnd = _kickHitEnd;
	}
	else
	{
		if (_comboIndex < 0 ||
			_comboIndex >=
			static_cast<int>(_pipeCombos.size()))
		{
			ChangeState(PlayerState::Idle);
			return;
		}

		const auto& combo = _pipeCombos[_comboIndex];

		animationName = combo.animation;
		hitStart = combo.hitStart;
		hitEnd = combo.hitEnd;
		soundProgress = combo.soundProgress;
	}

	float progress = 0.f;

	// 현재 공격 애니메이션이 확인된 경우에만 진행
	// 전환 중인 이전 애니메이션의 종료를 오인하지 않도록 함
	if (!animator->GetAnimationProgress(
		animationName,
		progress))
	{
		return;
	}

	const float previous = _previousAttackProgress;
	_previousAttackProgress = progress;

	// 파이프 휘두르는 소리: 각 타격에서 한 번
	if (!isKick &&
		!_attackSoundPlayed &&
		progress >= soundProgress)
	{
		_attackSoundPlayed = true;

		SoundManager::Get().PlaySFX("PipeSwing");
	}

	// 프레임이 타격 구간을 건너뛰어도 한 번 검사
	const bool crossedHitWindow =
		progress >= previous &&
		progress >= hitStart &&
		previous <= hitEnd;

	if (crossedHitWindow)
	{
		if (isKick)
		{
			ApplyKickDamage();
		}
		else if (_weapon)
		{
			_weapon->Attack();
		}
	}

	// 데미지 콜백에서 피격 등으로 상태가 바뀌었다면 중단
	if (_state != attackState ||
		_comboIndex != attackIndex)
	{
		return;
	}

	// 현재 공격이 끝날 때까지 대기
	if (progress < 0.99f ||
		!animator->IsAnimationFinished())
	{
		return;
	}

	// 다음 공격이 예약되어 있으면 Idle을 거치지 않고 연결
	if (!isKick &&
		_comboQueued &&
		_comboIndex + 1 <
		static_cast<int>(_pipeCombos.size()))
	{
		const int nextIndex = _comboIndex + 1;

		if (BeginPipeCombo(nextIndex))
			return;
	}

	ChangeState(PlayerState::Idle);
}

void Player::ResetAttack()
{
	if (_weapon)
		_weapon->EndAttack();

	_comboIndex = -1;
	_comboQueued = false;
	_attackSoundPlayed = false;
	_previousAttackProgress = 0.f;

	_kickTarget.reset();
	_kickHitDone = false;
}

