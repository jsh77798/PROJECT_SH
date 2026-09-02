#include "pch.h"
#include <map>
#include "Enemy.h"
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

Enemy::Enemy()
{
}

Enemy::~Enemy()
{
}

void Enemy::Init()
{
	InitCharacter();


	//////////////////////////// ResourceData ////////////////////////////

	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));
	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Idle.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Run.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Slash.fbx"));
	_animMap[EnemyState::Idle] = model->FindAnimation(L"Kachujin/Idle");
	_animMap[EnemyState::Move] = model->FindAnimation(L"Kachujin/Run");
	_animMap[EnemyState::Attack] = model->FindAnimation(L"Kachujin/Slash");
	//////////////////////////////////////////////////////////////////////

	
	// EnemyController
	auto enemyController = make_shared<EnemyController>();
	enemyController->SetEnemy(this);
	enemyController->SetTarget(_target);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.5f);

	// HealthComponent
	auto healthComponent = make_shared<HealthComponent>();

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.01f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, 0.0f });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// * Enemy *
	GetOrAddTransform()->SetPosition(Vec3{ -10.0f, 0.0f, 10.0f });
	GetCharacterMovement()->SetMoveSpeed(2.0f);
	AddComponent(enemyController);
	AddComponent(collider);
	AddComponent(healthComponent);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
	CUR_SCENE->Add(_modelObject);
}

void Enemy::Update()
{
    GameObject::Update();
}

void Enemy::ChangeState(EnemyState state)
{
	if (_state == state)
		return;

	_state = state;

	if (_modelObject) {

		auto animator = _modelObject->GetModelAnimator();

		if (animator) {
			animator->Play(_animMap[state]);
		}
	}
}

void Enemy::Move()
{
	ChangeState(EnemyState::Move);

}

void Enemy::Stop()
{
	ChangeState(EnemyState::Idle);
}

