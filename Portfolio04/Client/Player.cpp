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
	model->ReadModel(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Idle.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Run.fbx"));
	_animMap[PlayerState::Idle] = model->FindAnimation(L"Kachujin/Idle");
	_animMap[PlayerState::Move] = model->FindAnimation(L"Kachujin/Run");
	//////////////////////////////////////////////////////////////////////


	// PlayerController
	auto playerController = make_shared<PlayerController>();
	playerController->SetPlayer(this);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.5f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.01f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, 0.0f });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Camera
	auto camScript = make_shared<CameraScript>();
	camScript->SetTarget(shared_from_this());
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(camScript);
	_camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);

	// * Player *
	GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 0.0f, 0.0f });
	GetCharacterMovement()->SetMoveSpeed(5.0f);
	AddComponent(playerController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
	CUR_SCENE->Add(_modelObject);
	CUR_SCENE->Add(_camera);
}

void Player::Update()
{
    GameObject::Update();
}

void Player::ChangeState(PlayerState state)
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

void Player::Move()
{
	ChangeState(PlayerState::Move);

}

void Player::Stop()
{
	ChangeState(PlayerState::Idle);
}

