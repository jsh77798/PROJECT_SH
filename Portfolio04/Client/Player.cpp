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

Player::Player()
{
	map<State, shared_ptr<ModelAnimation>> Anim;

}

Player::~Player()
{
}

void Player::Init()
{
	shared_ptr<Shader> _shader = make_shared<Shader>(L"SkinnedLit.fx");

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

	// Model
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(L"Kachujin/Kachujin");
	model->ReadMaterial(L"Kachujin/Kachujin");

	// Animation
	anim[State::Idle] = ASSIMP->AnimImporter(L"Kachujin/Idle.fbx");
	
	// CharacterMesh
	auto obj = make_shared<GameObject>();
	obj->GetOrAddTransform()->SetPosition(Vec3{ 0.0f, 0.0f, 0.0f });
	obj->GetOrAddTransform()->SetScale(Vec3(0.01f));
	obj->AddComponent(make_shared<ModelAnimator>(_shader));
	//obj->AddComponent(make_shared<MeshRenderer>());
	obj->AddComponent(make_shared<PlayerController>());
	{
		//auto mesh = RESOURCES->Get<Mesh>(L"Sphere");
		//obj->GetMeshRenderer()->SetMesh(mesh);
		obj->GetModelAnimator()->SetModel(model);
	}

	// CameraScript
	auto camScript = make_shared<CameraScript>();
	camScript->SetTarget(obj);

	// Camera
	_camera = make_shared<GameObject>();
	_camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
	_camera->AddComponent(make_shared<Camera>());
	_camera->AddComponent(camScript);
	_camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);


	CUR_SCENE->Add(obj);
	CUR_SCENE->Add(_camera);
}

void Player::Update()
{
    GameObject::Update();
}