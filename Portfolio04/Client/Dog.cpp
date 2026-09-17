#include "pch.h"
#include <map>
#include "Dog.h"
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

Dog::Dog()
{
}

Dog::~Dog()
{
}

void Dog::Init()
{
	Enemy::Init();

	//////////////////////////// ResourceData ////////////////////////////

	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"Dog/Dog.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Dog/Dog.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"Dog/Dog_Idle.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Dog/Dog_Move.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Dog/Dog_Attack1.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Dog/Dog_Death.fbx"));
	//////////////////////////////////////////////////////////////////////


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
	_health->SetMaxHealth(1000.f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.00005f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, XM_PI });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Animation Data
	auto animator = _modelObject->GetModelAnimator();
	_animMap[EnemyState::Idle] = animator->MakeAnimData("Idle", model->FindAnimation(L"Dog/Dog_Idle"));
	_animMap[EnemyState::Move] = animator->MakeAnimData("Move", model->FindAnimation(L"Dog/Dog_Move"));
	_animMap[EnemyState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"Dog/Dog_Attack1"), false);
	_animMap[EnemyState::Dead] = animator->MakeAnimData("Dead", model->FindAnimation(L"Dog/Dog_Death"), false);

	// * Dog *
	GetOrAddTransform()->SetPosition(Vec3{ -5.0f, 5.0f, 5.0f });
	GetCharacterMovement()->SetMoveSpeed(2.0f);
	AddComponent(_enemyController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
}

void Dog::Update()
{
	Enemy::Update();
}
