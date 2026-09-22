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
#include "KeyItem.h"

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

        sound.LoadSFX(
            "ItemPickup",
            "../Resources/Sounds/SFX/SH-Menu-Blip-02.wav"
        );
        sound.LoadSFX(
            "PlayerHit",
            "../Resources/Sounds/SFX/SH_Harry_Grunt.wav"
        );


        // Dog
        sound.LoadSFX(
            "DogAttack",
            "../Resources/Sounds/SFX/SH-Demon-Dog-Bark.wav"
        );

        sound.LoadSFX(
            "DogDead",
            "../Resources/Sounds/SFX/SH-Demon-Dog-Kicked.wav"
        );


        // CLD1,2
        sound.LoadSFX(
            "CLDAttack",
            "../Resources/Sounds/SFX/SH_Weapons_Knife_Slash.wav"
        );

        sound.LoadSFX(
            "CLDDead",
            "../Resources/Sounds/SFX/SH-Demon-Child-Kicked.wav"
        );


        // CLD3
        sound.LoadSFX(
            "CLD3Attack",
            "../Resources/Sounds/SFX/SH-Demon-Bird-Squak.wav"
        );

        sound.LoadSFX(
            "CLD3Dead",
            "../Resources/Sounds/SFX/SH-Doctor-Moan.wav"
        );


        // DoorOpen
        //sound.LoadSFX(
        //    "DoorOpen",
        //    "../Resources/Sounds/SFX/SH-Door-Open-01.wav"
        //);
        sound.LoadSFX(
            "DoorWood",
            "../Resources/Sounds/SFX/SH-Door-Open-02.wav"
        );

        sound.LoadSFX(
            "DoorLargeWood",
            "../Resources/Sounds/SFX/SH-Door-Open-01.wav"
        );

        sound.LoadSFX(
            "DoorMetal",
            "../Resources/Sounds/SFX/SH-Door-Open-03.wav"
        );

        sound.LoadSFX(
            "DoorLocked",
            "../Resources/Sounds/SFX/SH-Door-Locked.wav"
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
    _mapShader = make_shared<Shader>(L"Map.fx");
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
        _skyShader =
            make_shared<Shader>(L"18. SkyDemo.fx");

        // 실제 하늘 텍스처 파일이 필요합니다.
        auto texture = RESOURCES->Load<Texture>(
            L"Sky3Texture",
            L"..\\Resources\\Textures\\sky3.png"
        );

        auto skybox = make_shared<Skybox>();
        skybox->Init(_skyShader, texture);

        CUR_SCENE->SetSkybox(skybox);
    }


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
    // Item 생성
    // ==========================
    for (const auto& point : spawnPoints)
    {
        std::string keyID;
        std::wstring modelFile;

        if (point.name == L"SPAWN_Key_Door03")
        {
            keyID = "Key_Door03";
            modelFile = L"KeyDoor03/Silent Hill 1 Meshes - House Key.fbx";
        }
        //else if (point.name == L"SPAWN_Key_Door06")
        //{
        //    keyID = "Key_Door06";
        //    modelFile = L"KeyDoor03/Silent Hill 1 Meshes - House Key.fbx";
        //}
        else
        {
            continue;
        }

        const Matrix itemWorld =
            point.transform * mapWorld;

        Vec3 position = XMVector3TransformCoord(
            Vec3::Zero,
            itemWorld
        );

        auto item = make_shared<KeyItem>();

        item->Init(
            keyID,
            modelFile
        );

        item->GetOrAddTransform()->SetPosition(position);

        CUR_SCENE->Add(item);
    }

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

            // ★ CLD3에만 사망 이벤트 연결
            if (auto cld3 = dynamic_pointer_cast<CLD3>(enemy))
            {
                cld3->SetOnDeathEvent(
                    [this]()
                    {
                        OnCLD3Defeated();
                    }
                );
            }

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

        doorTransition->SetOnTransitionCompleted(
            [this](
                const std::wstring& triggerName,
                const std::string& defaultBGM)
            {
                OnDoorTransitionCompleted(
                    triggerName,
                    defaultBGM
                );
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
    floor->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 100.f, 10.f });
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

    ApplyEnvironmentColors();
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

                // ★ 누워 있거나 죽은 척하는 적은 음악 판정에서 제외
                if (!enemy->CanTriggerTensionMusic())
                    continue;

                // ★ 각 적에게 설정된 거리 사용
                const float range =
                    _tensionActive
                    ? enemy->GetTensionLeaveRange()
                    : enemy->GetTensionEnterRange();

                const float rangeSquared =
                    range * range;

                const Vec3 difference =
                    enemy->GetTransform()->GetPosition()
                    - playerPosition;

                // 높이를 포함한 3차원 거리
                if (difference.LengthSquared() <=
                    rangeSquared)
                {
                    enemyNearby = true;
                    break;
                }
            }
        }
    }

    if (_cld3MusicActive || _cld3Defeated)
    {
        enemyNearby = false;
    }

    _tensionActive = enemyNearby;

    sound.SetTensionActive(_tensionActive);

    // 페이드 처리를 위해 계속 호출
    sound.Update(TIME->GetDeltaTime());
}

void TownScene::Render()
{
    Scene::Render();
}

void TownScene::ApplyEnvironmentColors()
{
    
    // 하늘: 기본 회색 또는 노을 텍스처
    if (_skyShader)
    {
        float normalSkyColor[4] =
        {
            0.4f, 0.4f, 0.4f, 1.f
        };

        auto skyColor =
            _skyShader->GetVector("SkyColor");

        if (skyColor && skyColor->IsValid())
        {
            skyColor->SetFloatVector(normalSkyColor);
        }

        auto useTexture =
            _skyShader->GetScalar("UseSkyTexture");

        if (useTexture && useTexture->IsValid())
        {
            useTexture->SetFloat(
                _sunsetActive ? 1.f : 0.f
            );
        }
    }

    // 안개: 기존 색상 유지
    if (_mapShader)
    {
        float fogColor[4] =
        {
            0.4f, 0.4f, 0.4f, 1.f
        };

        if (_sunsetActive)
        {
            fogColor[0] = 0.58f;
            fogColor[1] = 0.45f;
            fogColor[2] = 0.25f;
        }

        auto fog =
            _mapShader->GetVector("FogColor");

        if (fog && fog->IsValid())
        {
            fog->SetFloatVector(fogColor);
        }
    }
}

void TownScene::OnDoorTransitionCompleted(const std::wstring& triggerName, const std::string& defaultBGM)
{
    auto& sound = SoundManager::Get();

    // =========================
    // CLD3 처치 이후의 음악 규칙
    // =========================
    if (_cld3Defeated)
    {
        _cld3MusicActive = false;

        // 거리 기반 긴장 OST도 계속 무음 유지
        _tensionActive = false;
        sound.SetTensionActive(false, true);

        // 이미 새 음악을 시작했다면 다른 문에서도 유지
        if (_postCLD3MusicStarted)
            return;

        if (triggerName == L"DOOR_02_IN_TRIGGER")
        {
            _sunsetActive = true;
            ApplyEnvironmentColors();

            sound.SetBGMVolume(0.8f);

            // 실제 사용할 음악 경로로 변경
            _postCLD3MusicStarted = sound.PlayBGM(
                "../Resources/Sounds/BGM/SH-Disc2-20-AfterAll.wav"
            );
        }
        else
        {
            // 지정한 문을 통과하기 전에는 항상 무음
            sound.StopBGM();
        }

        return;
    }

    // =========================
    // CLD3 처치 이전의 기존 규칙
    // =========================
    _cld3MusicActive =
        (triggerName == L"DOOR_05_IN_TRIGGER");

    if (_cld3MusicActive)
    {
        _tensionActive = false;
        sound.SetTensionActive(false, true);

        sound.SetBGMVolume(0.75f);

        sound.PlayBGM(
            "../Resources/Sounds/BGM/"
            "SH-Disc2-02-ANewForm.wav"
        );

        return;
    }

    // 일반 구역 BGM
    sound.SetBGMVolume(0.35f);

    if (!defaultBGM.empty())
    {
        sound.PlayBGM(defaultBGM);
    }
    else
    {
        sound.StopBGM();
    }
}

void TownScene::OnCLD3Defeated()
{
    //if (_cld3Defeated)
    //    return;
    //
    //_cld3Defeated = true;
    //
    //// 다른 구역의 일반 음악은 끄지 않음
    //if (_cld3MusicActive)
    //{
    //    auto& sound = SoundManager::Get();
    //
    //    sound.StopBGM();
    //
    //    _tensionActive = false;
    //    sound.SetTensionActive(false, true);
    //}

    if (_cld3Defeated)
        return;

    _cld3Defeated = true;
    _cld3MusicActive = false;
    _postCLD3MusicStarted = false;

    auto& sound = SoundManager::Get();

    sound.StopBGM();

    _tensionActive = false;
    sound.SetTensionActive(false, true);
}
