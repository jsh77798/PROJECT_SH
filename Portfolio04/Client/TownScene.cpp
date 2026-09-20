#include "pch.h"
#include "TownScene.h"
#include "Map.h"
#include "DoorTransition.h"
#include "Player.h"
#include "Enemy.h"
#include "Dog.h"
#include "CLD1.h"
#include "CLD2.h"
#include "CLD3.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Light.h"
#include "Material.h"
#include "EnemyController.h"
#include "SnowBillboard.h"
#include "Skybox.h"
#include "Model.h"
#include "CharacterMovement.h"
#include "BaseCollider.h"
#include "ModelAnimator.h"
#include "SphereCollider.h"
#include "AABBBoxCollider.h"
#include "SoundManager.h"
#include "HealthComponent.h"
#include "Scene.h"

TownScene::TownScene()
    : mPlayer(nullptr)
{
}

TownScene::~TownScene()
{
}

void TownScene::Start()
{
    // ==========================
    // Sound
    // ==========================
    auto& sound = SoundManager::Get();

    if (sound.Init())
    {
        sound.SetMasterVolume(1.f);
        sound.SetBGMVolume(0.35f);
        sound.SetSFXVolume(0.8f);

        // Player
        sound.LoadSFX(
            "Footstep01",
            "../Resources/Sounds/SFX/SH_Footstep_Hard.wav",
            4
        );

        sound.LoadSFX(
            "Footstep02",
            "../Resources/Sounds/SFX/SH_Footstep_Hard.wav",
            4
        );

        sound.LoadSFX(
            "PipeSwing",
            "../Resources/Sounds/SFX/SH_Weapons_Pipe_Swing.wav"
        );

        sound.LoadSFX(
            "PipeHit",
            "../Resources/Sounds/SFX/SH-Demon-Bird-Hitting-the-Ground.wav"
        );



        // DoorOpen
        sound.LoadSFX(
            "DoorOpen",
            "../Resources/Sounds/SFX/SH-Door-Open-01.wav"
        );

        // BGM
        sound.PlayBGM(
            "../Resources/Sounds/BGM/SH-Disc1-08-NightmarishEnd.wav"
        );
        
        //sound.PlayBGM(
        //    "../Resources/Sounds/BGM/SH-Disc1-02-FogEnsues.wav"
        //);

        sound.LoadTensionBGM(
            "../Resources/Sounds/BGM/SH_Radio_01.wav"
        );
        sound.SetTensionVolume(0.55f);
        sound.SetTensionActive(false);
        _tensionActive = false;
    }




    _shader = make_shared<Shader>(L"SkinnedLit.fx");
    shared_ptr<Shader> _mapShader = make_shared<Shader>(L"Map.fx");
    shared_ptr<Shader> _debugShader = make_shared<Shader>(L"Debug.fx");

    // ==========================
    // Map 생성
    // ==========================
    auto map = make_shared<Map>();
    map->Init(_mapShader, _debugShader);
    CUR_SCENE->Add(map);
    CUR_SCENE->SetSnowEnabled(true);

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

    //// ==========================
    //// Player 생성
    //// ==========================
    //mPlayer = make_shared<Player>();
    //mPlayer->Init();
    //CUR_SCENE->Add(mPlayer);
    //
    //
    //// ==========================
    //// Enemy 생성
    //// ==========================
    //// Dog
    //auto mDog = make_shared<Dog>();
    //mDog->SetTarget(mPlayer);
    //mDog->Init();
    //CUR_SCENE->Add(mDog);
    //// CLD1
    //auto mCLD1 = make_shared<CLD1>();
    //mCLD1->SetTarget(mPlayer);
    //mCLD1->Init();
    //CUR_SCENE->Add(mCLD1);
    //// CLD2
    //auto mCLD2 = make_shared<CLD2>();
    //mCLD2->SetTarget(mPlayer);
    //mCLD2->Init();
    //CUR_SCENE->Add(mCLD2);
    //// CLD3
    //auto mCLD3 = make_shared<CLD3>();
    //mCLD3->SetTarget(mPlayer);
    //mCLD3->Init();
    //CUR_SCENE->Add(mCLD3);


    // ==========================
    // Blender 배치 지점으로 캐릭터 생성
    // ==========================
    const auto& spawnPoints =
        map->GetModel()->GetSpawnPoints();

    const Matrix mapWorld =
        map->GetTransform()->GetWorldMatrix();

    // 캐릭터의 위치와 방향 설정
    auto applySpawnPoint =
        [&](const shared_ptr<Character>& character,
            const ModelSpawnPoint& point)
        {
            // Empty의 모델 공간 행렬에 맵 월드 변환 적용
            const Matrix spawnWorld =
                point.transform * mapWorld;

            // Empty 원점 = 발이 닿는 위치
            Vec3 position = XMVector3TransformCoord(
                Vec3::Zero,
                spawnWorld
            );

            // 바닥 위치를 캐릭터 원점 위치로 보정
            auto movement = character->GetCharacterMovement();

            if (movement)
            {
                position.y +=
                    movement->GetGroundPlacementOffset();
            }

            auto transform =
                character->GetOrAddTransform();

            transform->SetPosition(position);

            // 배치 지점의 로컬 +Z를 엔진에서의 전방으로 사용
            Vec3 forward = XMVector3TransformNormal(
                Vec3(0.f, 1.f, 0.f),
                spawnWorld
            );

            forward.y = 0.f;

            if (forward.LengthSquared() > 0.000001f)
            {
                forward.Normalize();

                const float yaw =
                    std::atan2(forward.x, forward.z);

                // 캐릭터는 바닥에 똑바로 서 있도록 Y축 회전만 적용
                transform->SetLocalRotation(
                    Vec3(0.f, yaw, 0.f)
                );
            }

            // 맵의 스케일을 캐릭터 스케일에 복사하지 않음.
            // 캐릭터 모델 크기는 각 Init()의 설정을 사용.

            if (auto collider = character->GetCollider())
            {
                collider->Update();
            }
        };

    // ==========================
    // 1. Player 생성
    // ==========================
    const ModelSpawnPoint* playerSpawn = nullptr;

    for (const auto& point : spawnPoints)
    {
        if (point.name == L"SPAWN_Player")
        {
            playerSpawn = &point;
            break;
        }
    }

    if (playerSpawn)
    {
        mPlayer = make_shared<Player>();
        mPlayer->Init();

        // Init 내부의 기본 위치보다 나중에 적용
        applySpawnPoint(mPlayer, *playerSpawn);

        CUR_SCENE->Add(mPlayer);
    }
    else
    {
        OutputDebugStringA(
            "TownScene: SPAWN_Player was not found. "
            "Character spawning skipped.\n"
        );
    }

    // ==========================
    // 2. Enemy 생성
    // ==========================
    if (mPlayer)
    {
        for (const auto& point : spawnPoints)
        {
            shared_ptr<Enemy> enemy;

            if (point.name.rfind(L"SPAWN_Dog_", 0) == 0)
            {
                enemy = make_shared<Dog>();
            }
            else if (point.name.rfind(L"SPAWN_CLD1_", 0) == 0)
            {
                enemy = make_shared<CLD1>();
            }
            else if (point.name.rfind(L"SPAWN_CLD2_", 0) == 0)
            {
                enemy = make_shared<CLD2>();
            }
            else if (point.name.rfind(L"SPAWN_CLD3_", 0) == 0)
            {
                enemy = make_shared<CLD3>();
            }
            else
            {
                // Player 지점 및 지원하지 않는 이름
                continue;
            }

            // Init에서 컨트롤러에 전달할 타깃
            enemy->SetTarget(mPlayer);

            // virtual Init이므로 Dog/CLD별 Init 실행
            enemy->Init();

            applySpawnPoint(enemy, point);

            CUR_SCENE->Add(enemy);
        }
    }


    if (mPlayer)
    {
        auto doorObject = make_shared<GameObject>();
        doorObject->GetOrAddTransform();

        auto doorTransition =
            make_shared<DoorTransition>();

        weak_ptr<Player> weakPlayer = mPlayer;

        doorTransition->Init(
            mPlayer,
            map->GetModel(),
            map->GetTransform()->GetWorldMatrix(),
            [weakPlayer]()
            {
                if (auto player = weakPlayer.lock())
                {
                    player->ResetCameraAfterTeleport();
                }
            }
        );

        doorObject->AddComponent(doorTransition);
        CUR_SCENE->Add(doorObject);
    }


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
    Scene::Update();

    auto& sound = SoundManager::Get();

    bool enemyNearby = false;

    if (mPlayer)
    {
        auto playerHealth =
            mPlayer->GetHealthComponent();

        if (playerHealth && !playerHealth->IsDead())
        {
            const Vec3 playerPosition =
                mPlayer->GetTransform()->GetPosition();

            // 활성화 중에는 더 멀어져야 해제
            const float range =
                _tensionActive
                ? _tensionLeaveRange
                : _tensionEnterRange;

            const float rangeSquared = range * range;

            for (const auto& object :
                CUR_SCENE->GetObjects())
            {
                auto enemy =
                    dynamic_pointer_cast<Enemy>(object);

                if (!enemy)
                    continue;

                auto health =
                    enemy->GetHealthComponent();

                if (!health ||
                    health->IsDead() ||
                    enemy->IsDead())
                {
                    continue;
                }

                const Vec3 difference =
                    enemy->GetTransform()->GetPosition()
                    - playerPosition;

                // 높이 차이도 포함한 3차원 거리
                if (difference.LengthSquared() <=
                    rangeSquared)
                {
                    enemyNearby = true;
                    break;
                }
            }
        }
    }

    _tensionActive = enemyNearby;

    sound.SetTensionActive(_tensionActive);

    // 매 프레임 한 번만 호출
    sound.Update(TIME->GetDeltaTime());
}

void TownScene::Render()
{
    Scene::Render();
}