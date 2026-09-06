#include "pch.h"
#include "TownScene.h"
#include "Player.h"
#include "Enemy.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Light.h"
#include "Material.h"
#include "EnemyController.h"

#include "ModelAnimator.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"

TownScene::TownScene()
    : mPlayer(nullptr)
{
}

TownScene::~TownScene()
{
}

void TownScene::Start()
{
    _shader = make_shared<Shader>(L"SkinnedLit.fx");
    shared_ptr<Shader> _debugShader = make_shared<Shader>(L"Debug.fx");

    // ==========================
    // Player 积己
    // ==========================
    mPlayer = make_shared<Player>();
    mPlayer->Init();
    CUR_SCENE->Add(mPlayer);


    // ==========================
    // Enemy 积己
    // ==========================
    mEnemy = make_shared<Enemy>();
    mEnemy->SetTarget(mPlayer);
    mEnemy->Init();
    //mEnemy->Update();
    CUR_SCENE->Add(mEnemy);


    // ==========================
    // Light 积己
    // ==========================
    auto light = make_shared<GameObject>();
    light->AddComponent(make_shared<Light>());
    LightDesc lightDesc;
    lightDesc.ambient = Vec4(0.4f);
    lightDesc.diffuse = Vec4(1.f);
    lightDesc.specular = Vec4(0.1f);
    lightDesc.direction = Vec3(1.f, 0.f, 1.f);
    light->GetLight()->SetLightDesc(lightDesc);
    CUR_SCENE->Add(light);


    // ==========================
    // 抛胶飘侩 寒 积己
    // ==========================
    auto meshShader = make_shared<Shader>(L"23. RenderDemo.fx");
    
    // Material
    {
        shared_ptr<Material> material = make_shared<Material>();
		material->SetShader(meshShader);
		auto texture = RESOURCES->Load<Texture>(L"Veigar", L"..\\Resources\\Textures\\veigar.jpg");
		material->SetDiffuseMap(texture);
		MaterialDesc& desc = material->GetMaterialDesc();
		desc.ambient = Vec4(1.f);
		desc.diffuse = Vec4(1.f);
		desc.specular = Vec4(1.f);
		RESOURCES->Add(L"Veigar", material);
    }
    // floor
    auto collider = make_shared<AABBBoxCollider>(_debugShader);
    collider->SetExtents(
        Vec3(0.5f, 1.0f, 0.5f)
    );

    //collider->SetRadius(0.5f);

    auto floor = make_shared<GameObject>();
    floor->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.5f, 10.f });
    floor->GetOrAddTransform()->SetScale(Vec3{ 1.0f, 1.0f, 1.0f });
    floor->AddComponent(make_shared<MeshRenderer>());
    floor->AddComponent(collider);
    {
        auto mesh = RESOURCES->Get<Mesh>(L"Sphere");
        floor->GetMeshRenderer()->SetMesh(mesh);
    }
    {
        floor->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
    }
    CUR_SCENE->Add(floor);

}

void TownScene::Update()
{
}

void TownScene::Render()
{
}