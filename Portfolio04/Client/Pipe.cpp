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
#include "Enemy.h"
#include "SoundManager.h"

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

        if (health == nullptr || health->IsDead())
            continue;

        // 데미지 처리 전에 등록
        _hitObjects.insert(object);


        // 사운드 Hit
        SoundManager::Get().PlaySFX("PipeHit");

     
        // 맞은 대상의 실제 콜라이더 중심
        Vec3 targetCenter =
            character->GetTransform()->GetPosition();

        float surfaceOffset = 0.f;

        switch (collider->GetColliderType())
        {
        case ColliderType::Sphere:
        {
            auto sphere =
                dynamic_pointer_cast<SphereCollider>(collider);

            const auto& bounds = sphere->GetBoundingSphere();

            targetCenter = bounds.Center;
            surfaceOffset = bounds.Radius;
            break;
        }

        case ColliderType::AABB:
        {
            auto box =
                dynamic_pointer_cast<AABBBoxCollider>(collider);

            targetCenter = box->GetBoundingBox().Center;
            break;
        }

        case ColliderType::OBB:
        {
            auto box =
                dynamic_pointer_cast<OBBBoxCollider>(collider);

            targetCenter = box->GetBoundingBox().Center;
            break;
        }
        }

        // 타격받은 몸에서 공격자 쪽으로 피가 튀도록 설정
        Vec3 sprayDirection =
            owner->GetTransform()->GetPosition() - targetCenter;

        sprayDirection.y = 0.f;

        if (sprayDirection.LengthSquared() < 0.000001f)
        {
            sprayDirection =
                -owner->GetTransform()->GetForward();

            sprayDirection.y = 0.f;
        }

        if (sprayDirection.LengthSquared() < 0.000001f)
            sprayDirection = Vec3(0.f, 0.f, 1.f);

        sprayDirection.Normalize();

        // 구형 콜라이더는 공격자 쪽 표면에서 생성
        Vec3 hitPosition =
            targetCenter +
            sprayDirection * (surfaceOffset + 0.02f);

        // 사망 콜백으로 대상이 제거되기 전에 생성
        CUR_SCENE->SpawnBlood(
            hitPosition,
            sprayDirection
        );

        health->TakeDamage(
            GetDamage(),
            owner->GetTransform()->GetPosition()
        );

       
    }
}

