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
	model->ReadModel(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Kachujin/Mesh.fbx"));

	// Animation
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Idle.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Run.fbx"));
	model->ReadAnimation(ASSIMP->AnimImporter(L"Kachujin/Slash.fbx"));
	//////////////////////////////////////////////////////////////////////


	// PlayerController
	auto playerController = make_shared<PlayerController>();
	playerController->SetPlayer(this);

	// Collider
	auto collider = make_shared<SphereCollider>(_debugShader);
	collider->SetRadius(0.5f);

	// Health
	_health->SetMaxHealth(1000.f);

	// ModelObject
	_modelObject = make_shared<GameObject>();
	_modelObject->GetOrAddTransform()->SetScale(Vec3(0.01f));
	_modelObject->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, XM_PI, 0.0f });
	_modelObject->AddComponent(make_shared<ModelAnimator>(_shader));
	_modelObject->GetModelAnimator()->SetModel(model);

	// Animation Data
	auto animator = _modelObject->GetModelAnimator();
	_animMap[PlayerState::Idle] = animator->MakeAnimData("Idle", model->FindAnimation(L"Kachujin/Idle"));
	_animMap[PlayerState::Move] = animator->MakeAnimData("Move", model->FindAnimation(L"Kachujin/Run"));
	_animMap[PlayerState::Attack] = animator->MakeAnimData("Attack", model->FindAnimation(L"Kachujin/Slash"), false);

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
	socket->SetBoneName(L"RightHand");
	socket->SetAnimator(_modelObject->GetModelAnimator());
	_weaponSocket = make_shared<GameObject>();
	_weaponSocket->GetOrAddTransform();
	_weaponSocket->AddComponent(socket);
	// ★ Player의 자식으로 등록
	_modelObject->AddChild(_weaponSocket);

	// Pipe
	auto pipe = make_shared<Pipe>();
	pipe->Init();
	pipe->GetOrAddTransform()->SetLocalPosition(Vec3(0.f, 0.f, 0.f));
    pipe->GetOrAddTransform()->SetScale(Vec3(5.f));
	pipe->GetOrAddTransform()->SetRotation(Vec3{ 0.0f, 0.0f, 0.0f });
    EquipWeapon(pipe);

	// * Player *
	GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 0.0f, 0.0f });
	GetCharacterMovement()->SetMoveSpeed(5.0f);
	AddComponent(playerController);
	AddComponent(collider);
	AddChild(_modelObject); // Add ModelObject as a child of PlayerObject
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
	ChangeState(PlayerState::Move);
}

void Player::Stop()
{
	ChangeState(PlayerState::Idle);
}

//이후 무기 시스템을 만드면 Attack시 무리와 연동하여 무기에서 데미지를 입히도록 변경해야함
void Player::Attack()
{
	ChangeState(PlayerState::Attack);

	Ray ray;

	ray.position = GetTransform()->GetPosition();
	ray.direction = GetTransform()->GetForward();

	shared_ptr<BaseCollider> Collider = GetCollider();

	shared_ptr<BaseCollider> hitCollider;
	float hitDistance = 0.f;

	//일단은 선으로 RayCast해서 맞은 Collider가 Character인지 확인하고 맞으면 데미지 입히기
	if (SCENE->GetCurrentScene()->RayCast(
		ray,
		Collider,
		hitCollider,
		hitDistance) == false)
	{
		return;
	}

	shared_ptr<GameObject> hitObject = hitCollider->GetGameObject();
	Character* character = dynamic_cast<Character*>(hitObject.get());

	if (character == nullptr)
		return;

	auto health = character->GetHealthComponent();

	if (health == nullptr)
		return;

	health->TakeDamage(100.f);
}

void Player::EquipWeapon(shared_ptr<Weapon> weapon)
{
	if (weapon == nullptr)
		return;

	if (_weapon != nullptr)
	{
		// 기존 무기 제거
		// 일단은 나중에 구현
	}

	_weapon = weapon;

	_weaponSocket->AddChild(_weapon);
}

