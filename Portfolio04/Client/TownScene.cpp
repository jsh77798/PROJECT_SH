#include "pch.h"
#include "TownScene.h"
#include "Map.h"
#include "Player.h"
#include "Enemy.h"
#include "Dog.h"
#include "CLD1.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Light.h"
#include "Material.h"
#include "EnemyController.h"
#include "SnowBillboard.h"
#include "Skybox.h"

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
    shared_ptr<Shader> _mapShader = make_shared<Shader>(L"Map.fx");
    shared_ptr<Shader> _debugShader = make_shared<Shader>(L"Debug.fx");

    // ==========================
    // Map 생성
    // ==========================
    auto map = make_shared<Map>();
    map->Init(_mapShader, _debugShader);
    CUR_SCENE->Add(map);

    // ==========================
    // 환경 설정
    // ==========================
    // 눈 생성
    {
        auto shader =
            make_shared<Shader>(L"29. SnowDemo.fx");

        auto material = make_shared<Material>();
        material->SetShader(shader);

        auto texture = RESOURCES->Load<Texture>(
            L"SnowTexture",
            L"..\\Resources\\Textures\\snow.png"
        );

        material->SetDiffuseMap(texture);

        auto snowObject = make_shared<GameObject>();
        snowObject->GetOrAddTransform()
            ->SetLocalPosition(Vec3::Zero);

        auto snow = make_shared<SnowBillboard>(
            Vec3(40.f, 12.f, 40.f), // 눈이 내리는 영역 전체 크기
            1000                   // 눈송이 개수
        );

        snow->SetMaterial(material);
        snowObject->AddComponent(snow);

        CUR_SCENE->Add(snowObject);
    }
    // Skybox 생성
    {
        // 앞서 수정한 하늘 셰이더를 이 이름으로 저장
        auto shader =
            make_shared<Shader>(L"18. SkyDemo.fx");

        // 실제 하늘 텍스처 파일이 필요합니다.
        auto texture = RESOURCES->Load<Texture>(
            L"SkyTexture",
            L"..\\Resources\\Textures\\sky.png"
        );

        auto skybox = make_shared<Skybox>();
        skybox->Init(shader, texture);

        CUR_SCENE->SetSkybox(skybox);
    }

    // ==========================
    // Player 생성
    // ==========================
    mPlayer = make_shared<Player>();
    mPlayer->Init();
    CUR_SCENE->Add(mPlayer);


    // ==========================
    // Enemy 생성
    // ==========================
    // Dog
    auto mDog = make_shared<Dog>();
    mDog->SetTarget(mPlayer);
    mDog->Init();
    CUR_SCENE->Add(mDog);
    // CLD1
    auto mCLD1 = make_shared<CLD1>();
    mCLD1->SetTarget(mPlayer);
    mCLD1->Init();
    CUR_SCENE->Add(mCLD1);


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


    // ==========================
    // 테스트용 벽 생성
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