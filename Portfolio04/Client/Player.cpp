#include "pch.h"
#include "Player.h"
#include "PlayerController.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"
#include "CameraScript.h"
#include "Material.h"

Player::Player()
{
}

Player::~Player()
{
}

void Player::Init()
{
	shared_ptr<Shader> _shader = make_shared<Shader>(L"23. RenderDemo.fx");

	// Material
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(_shader);
		auto texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		material->SetDiffuseMap(texture);
		MaterialDesc& desc = material->GetMaterialDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);
		RESOURCES->Add(L"Veigar", material);
	}

	// CharacterMesh
	auto cube = make_shared<GameObject>();
	cube->GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 0.0f, 0.0f });
	cube->AddComponent(make_shared<MeshRenderer>());
	cube->AddComponent(make_shared<PlayerController>());
	{
		auto mesh = RESOURCES->Get<Mesh>(L"Sphere");
		cube->GetMeshRenderer()->SetMesh(mesh);
	}
	{
		cube->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
	}

	CUR_SCENE->Add(cube);

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