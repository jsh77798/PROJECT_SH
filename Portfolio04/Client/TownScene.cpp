#include "pch.h"
#include "TownScene.h"
#include "Player.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Light.h"
#include "Material.h"

TownScene::TownScene()
    : mPlayer(nullptr)
{
}

TownScene::~TownScene()
{
}

void TownScene::Start()
{
    _shader = make_shared<Shader>(L"23. RenderDemo.fx");

    // ==========================
    // Player 생성
    // ==========================
    mPlayer = make_shared<Player>();
    mPlayer->Init();


    // ==========================
    // Light 생성
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

    // ==========================
    // 테스트용 바닥 생성
    // ==========================
    auto floor = make_shared<GameObject>();
    floor->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, 0.f });
    floor->GetOrAddTransform()->SetScale(Vec3{ 20.0f, 0.1f, 20.0f });
    floor->AddComponent(make_shared<MeshRenderer>());
    {
        auto mesh = RESOURCES->Get<Mesh>(L"Quad");
        floor->GetMeshRenderer()->SetMesh(mesh);
    }
    {
        floor->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
    }
    CUR_SCENE->Add(floor);

    // ==========================
    // 테스트용 큐브
    // ==========================
    auto cube = make_shared<GameObject>();
    cube->GetOrAddTransform()->SetPosition(Vec3{ 3.0f, 0.0f, 3.0f });
    cube->AddComponent(make_shared<MeshRenderer>());
    {
        auto mesh = RESOURCES->Get<Mesh>(L"Sphere");
        cube->GetMeshRenderer()->SetMesh(mesh);
    }
    {
        cube->GetMeshRenderer()->SetMaterial(RESOURCES->Get<Material>(L"Veigar"));
    }
    CUR_SCENE->Add(cube);
}

void TownScene::Update()
{
}

void TownScene::Render()
{
}