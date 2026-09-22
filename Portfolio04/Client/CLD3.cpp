#include "pch.h"
#include <map>
#include "CLD3.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Material.h"
#include "Model.h"
#include "ModelAnimator.h"
#include "AssetImporter.h"
#include "SphereCollider.h"
#include "CharacterMovement.h"
#include "EnemyController.h"
#include "HealthComponent.h"
#include "KeyItem.h"
#include "Scene.h"

CLD3::CLD3()
{
}

CLD3::~CLD3()
{
}

void CLD3::Init()
{
	Enemy::Init();

	//////////////////////////// ResourceData ////////////////////////////

	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"CLD3/CLD3.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"CLD3/CLD3.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Idle1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Idle2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Idle3.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Stand1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Stand2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Move1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Attack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Hit1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Hit2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Hit3.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Death1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD3/CLD3_Death2.fbx"));
	//////////////////////////////////////////////////////////////////////


	_attackCooldown = 0.7f;

	_isLying = true;
	_hasAwakened = false;

	// Sound
	_attackSound = "CLD3Attack";
	_deathSound = "CLD3Dead";

	// Movement
	_movement->SetFootOffset(0.f);

	// EnemyController
	_enemyController = make_shared<EnemyController>();
	_enemyController->SetEnemy(this);
	_enemyController->SetTarget(_target);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.7f);
	collider->SetCenterOffset(Vec3(0.f, 0.5f, 0.f));

	// Health
	_health->SetMaxHealth(1900.f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.00008f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Animation Data
	auto animator = _modelObject->GetModelAnimator();
	_animMap[EnemyState::Idle] = animator->MakeAnimData("Idle", model->FindAnimation(L"CLD3/CLD3_Idle1"));
	_awakeIdleAnimation = animator->MakeAnimData("AwakeIdle", model->FindAnimation(L"CLD3/CLD3_Idle3"));
	_animMap[EnemyState::WakeUp] = animator->MakeAnimData("WakeUp", model->FindAnimation(L"CLD3/CLD3_Stand1"), false);
	_animMap[EnemyState::LieDown] = animator->MakeAnimData("LieDown", model->FindAnimation(L"CLD3/CLD3_Death1"), false);
	_animMap[EnemyState::Move] = animator->MakeAnimData("Move", model->FindAnimation(L"CLD3/CLD3_Move1"));
	_animMap[EnemyState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"CLD3/CLD3_Attack1"), false);
	_animMap[EnemyState::Dead] = animator->MakeAnimData("Dead", model->FindAnimation(L"CLD3/CLD3_Death1"), false);
	_animMap[EnemyState::Hit] = animator->MakeAnimData("Hit", model->FindAnimation(L"CLD3/CLD3_Hit3"), false);
	_animMap[EnemyState::Thanatosis] = animator->MakeAnimData("Thanatosis", model->FindAnimation(L"CLD3/CLD3_Idle1"), false);
	_ThanatosisHitAnimation = animator->MakeAnimData("ThanaHit", model->FindAnimation(L"CLD3/CLD3_Hit1"), false);

	// * CLD3 *
	GetOrAddTransform()->SetPosition(Vec3{ 5.0f, 5.0f, 2.0f });
	GetCharacterMovement()->SetMoveSpeed(2.0f);
	AddComponent(_enemyController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
}

void CLD3::Update()
{
	auto health = GetHealthComponent();

	if (!_keyDropped && health && health->IsDead())
	{
		DropKey();
	}

	Enemy::Update();
}

void CLD3::DropKey()
{
	if (_keyDropped)
		return;

	auto movement = GetCharacterMovement();

	if (!movement)
		return;

	// 키의 생성 위치는 몬스터의 발 위치
	const Vec3 dropPosition =
		movement->GetFootPosition();

	auto key = make_shared<KeyItem>();

	// 두 번째 인자는 기존 Door06 키에 사용하던
	// 실제 FBX 경로로 맞춰주세요.
	key->Init(
		"Key_Door06",
		L"KeyDoor03/Silent Hill 1 Meshes - House Key.fbx"
	);

	key->SetVisualHeight(1.f);

	key->GetOrAddTransform()->SetPosition(
		dropPosition
	);

	// 씬에 등록하기 전에 중복 드랍 방지
	_keyDropped = true;

	CUR_SCENE->Add(key);

	OutputDebugStringA(
		"[CLD3] Dropped Key_Door06\n"
	);
}
