#include "pch.h"
#include "Pipe.h"
#include "AssetImporter.h"
#include "Character.h"
#include "HealthComponent.h"
#include "Model.h"
#include "ModelRenderer.h"

Pipe::Pipe()
{
}

Pipe::~Pipe()
{
}

void Pipe::Init()
{
	auto _shader = make_shared<Shader>(L"23. RenderDemo.fx");


	//////////////////////////// ResourceData ////////////////////////////

	// Model (Mesh + Material)
	shared_ptr<class Model> model = make_shared<Model>();
	model->ReadModel(ASSIMP->MeshImporter(L"Tower/Tower.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Tower/Tower.fbx"));
	//////////////////////////////////////////////////////////////////////


	auto renderer =
		make_shared<ModelRenderer>(_shader);

	renderer->SetModel(model);

	AddComponent(renderer);
}

void Pipe::Update()
{
    GameObject::Update();
}

void Pipe::Attack()
{
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

