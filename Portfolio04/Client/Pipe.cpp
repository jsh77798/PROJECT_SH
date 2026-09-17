#include "pch.h"
#include "Pipe.h"
#include "AssetImporter.h"
#include "Character.h"
#include "HealthComponent.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "Scene.h"
#include "Transform.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "OBBBoxCollider.h"

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
	model->ReadModel(ASSIMP->MeshImporter(L"Pipe/Pipe.fbx"));
	model->ReadMaterial(ASSIMP->MeshImporter(L"Pipe/Pipe.fbx"));
	//////////////////////////////////////////////////////////////////////


	auto renderer = make_shared<ModelRenderer>(_shader);
	renderer->SetModel(model);
	renderer->SetPass(1);
	AddComponent(renderer);

    SetHitRadius(0.9f);
}

void Pipe::Update()
{
    GameObject::Update();
}

void Pipe::Attack()
{
    if (!_attackActive)
        return;

    auto owner = _owner.lock();

    if (owner == nullptr)
        return;

    BoundingSphere hitSphere;

    // 일단 파이프 오브젝트 원점을 중심으로 검사
    hitSphere.Center = GetTransform()->GetPosition();
    hitSphere.Radius = _hitRadius;

    for (const auto& object :
        SCENE->GetCurrentScene()->GetObjects())
    {
        if (object == owner || object.get() == this)
            continue;

        if (_hitObjects.find(object) != _hitObjects.end())
            continue;

        auto character = dynamic_pointer_cast<Character>(object);

        if (character == nullptr)
            continue;

        auto collider = object->GetCollider();

        if (collider == nullptr)
            continue;

        collider->Update();

        bool hit = false;

        switch (collider->GetColliderType())
        {
        case ColliderType::Sphere:
            hit = hitSphere.Intersects(
                dynamic_pointer_cast<SphereCollider>(collider)
                ->GetBoundingSphere()
            );
            break;

        case ColliderType::AABB:
            hit = hitSphere.Intersects(
                dynamic_pointer_cast<AABBBoxCollider>(collider)
                ->GetBoundingBox()
            );
            break;

        case ColliderType::OBB:
            hit = hitSphere.Intersects(
                dynamic_pointer_cast<OBBBoxCollider>(collider)
                ->GetBoundingBox()
            );
            break;
        }

        if (!hit)
            continue;

        auto health = character->GetHealthComponent();

        if (health == nullptr)
            continue;

        // 데미지 처리 전에 등록
        _hitObjects.insert(object);

        health->TakeDamage(GetDamage());
    }
}

