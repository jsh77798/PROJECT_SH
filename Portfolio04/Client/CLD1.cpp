#include "pch.h"
#include <map>
#include "CLD1.h"
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

CLD1::CLD1()
{
}

CLD1::~CLD1()
{
}

void CLD1::Init()
{
	Enemy::Init();

	//////////////////////////// ResourceData ////////////////////////////

	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"CLD1/CLD1.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"CLD1/CLD1.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Idle1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Idle2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Idle3.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Stand1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Stand2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Move1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Attack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Hit1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Hit2.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Hit3.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Death1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"CLD1/CLD1_Death2.fbx"));
	//////////////////////////////////////////////////////////////////////


	_attackCooldown = 0.75f;

	_isLying = true;
	_hasAwakened = false;

	// Sound
	_attackSound = "CLDAttack";
	_deathSound = "CLDDead";
	_tensionEnterRange = 10.f;
	_tensionLeaveRange = 12.f;

	// Movement
	_movement->SetFootOffset(0.f);

	// EnemyController
	_enemyController = make_shared<EnemyController>();
	_enemyController->SetEnemy(this);
	_enemyController->SetTarget(_target);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.5f);
	collider->SetCenterOffset(Vec3(0.f, 0.5f, 0.f));

	// Health
	_health->SetMaxHealth(1250.f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.00005f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Animation Data
	auto animator = _modelObject->GetModelAnimator();
	_animMap[EnemyState::Idle] = animator->MakeAnimData("Idle", model->FindAnimation(L"CLD1/CLD1_Idle1"));
	_awakeIdleAnimation = animator->MakeAnimData("AwakeIdle",model->FindAnimation(L"CLD1/CLD1_Idle3"));
	_animMap[EnemyState::WakeUp] = animator->MakeAnimData("WakeUp", model->FindAnimation(L"CLD1/CLD1_Stand1"), false);
	_animMap[EnemyState::LieDown] = animator->MakeAnimData("LieDown", model->FindAnimation(L"CLD1/CLD1_Death1"), false);
	_animMap[EnemyState::Move] = animator->MakeAnimData("Move", model->FindAnimation(L"CLD1/CLD1_Move1"));
	_animMap[EnemyState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"CLD1/CLD1_Attack1"), false);
	_animMap[EnemyState::Dead] = animator->MakeAnimData("Dead", model->FindAnimation(L"CLD1/CLD1_Death1"), false);
	_animMap[EnemyState::Hit] = animator->MakeAnimData("Hit", model->FindAnimation(L"CLD1/CLD1_Hit3"), false);
	_animMap[EnemyState::Thanatosis] = animator->MakeAnimData("Thanatosis", model->FindAnimation(L"CLD1/CLD1_Idle1"), false);
	_ThanatosisHitAnimation = animator->MakeAnimData("ThanaHit", model->FindAnimation(L"CLD1/CLD1_Hit1"), false);

	// * CLD1 *
	GetOrAddTransform()->SetPosition(Vec3{ -5.0f, 5.0f, 2.0f });
	GetCharacterMovement()->SetMoveSpeed(2.0f);
	AddComponent(_enemyController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
}

void CLD1::Update()
{
	Enemy::Update();
}
