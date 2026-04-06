#include "pch.h"
#include "Player.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "CameraScript.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Init()
{
	// Object
	_obj = make_shared<GameObject>();
	_obj->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, 0.f });
	_obj->AddComponent(make_shared<MeshRenderer>());
	{
		auto mesh = RESOURCES->Get<Mesh>(L"Sphere");
		_obj->GetMeshRenderer()->SetMesh(mesh);
	}
	CUR_SCENE->Add(_obj);

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(make_shared<CameraScript>());
	_camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);
	CUR_SCENE->Add(_camera);
}

void Player::Update()
{
    GameObject::Update();
}