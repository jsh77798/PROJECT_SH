#include "pch.h"
#include "DoorTransition.h"
#include "Player.h"
#include "Model.h"
#include "CharacterMovement.h"
#include "Scene.h"
#include <cmath>
#include <unordered_map>
#include "SoundManager.h"

void DoorTransition::Init(
    shared_ptr<Player> player,
    shared_ptr<Model> model,
    const Matrix& mapWorld,
    std::function<void()> resetCamera)
{
    _player = player;
    _resetCamera = resetCamera;

    _links.clear();
    _phase = Phase::Idle;
    _alpha = 0.f;
    _waitUntilOutside = false;

    if (!model)
        return;

    unordered_map<wstring, Matrix> points;

    for (const auto& point : model->GetDoorPoints())
    {
        points[point.name] =
            point.transform * mapWorld;
    }

    const wstring suffix = L"_OUT_TRIGGER";

    for (const auto& entry : points)
    {
        const wstring& name = entry.first;

        if (name.size() < suffix.size())
            continue;

        if (name.compare(
            name.size() - suffix.size(),
            suffix.size(),
            suffix) != 0)
        {
            continue;
        }

        // 예: DOOR_01
        const wstring prefix =
            name.substr(0, name.size() - suffix.size());

        auto outTrigger =
            points.find(prefix + L"_OUT_TRIGGER");
        auto inExit =
            points.find(prefix + L"_IN_EXIT");
        auto inTrigger =
            points.find(prefix + L"_IN_TRIGGER");
        auto outExit =
            points.find(prefix + L"_OUT_EXIT");

        if (outTrigger == points.end() ||
            inExit == points.end() ||
            inTrigger == points.end() ||
            outExit == points.end())
        {
            wstring message =
                L"Door points incomplete: " + prefix + L"\n";

            OutputDebugStringW(message.c_str());
            continue;
        }

        const std::string outsideBGM =
            "../Resources/Sounds/BGM/SH-Disc1-08-NightmarishEnd.wav";

        const std::string insideBGM =
            "../Resources/Sounds/BGM/SH-Disc1-02-FogEnsues.wav";

        // 기본값: 실내 방 사이의 문
        std::string inSideBGM = insideBGM;
        std::string outSideBGM = insideBGM;

        bool inSideIndoor = true;
        bool outSideIndoor = true;

        // 건물 출입문: 실제 사용하는 문 번호로 지정
        if (prefix == L"DOOR_01")
        {
            inSideBGM = insideBGM;
            outSideBGM = outsideBGM;

            inSideIndoor = true;
            outSideIndoor = false;
        }

        // 문 종류별 소리
        std::string openSound = "DoorWood";

        if (prefix == L"DOOR_02" ||
            prefix == L"DOOR_05" ||
            prefix == L"DOOR_06")
        {
            openSound = "DoorLargeWood";
        }
        else if (prefix == L"DOOR_03" ||
            prefix == L"DOOR_04")
        {
            openSound = "DoorMetal";
        }

        // 문별 필요한 열쇠
        std::string requiredKey;

        if (prefix == L"DOOR_03")
        {
            requiredKey = "Key_Door03";
        }
        else if (prefix == L"DOOR_06")
        {
            requiredKey = "Key_Door06";
        }

        // OUT -> IN
        AddLink(
            outTrigger->second,
            inExit->second,
            inSideBGM,
            inSideIndoor,
            openSound,
            requiredKey
        );

        // IN -> OUT
        AddLink(
            inTrigger->second,
            outExit->second,
            outSideBGM,
            outSideIndoor,
            openSound,
            requiredKey
        );
    }

    if (_links.empty())
    {
        OutputDebugStringA(
            "DoorTransition: no complete door links.\n"
        );
    }

    char buffer[128];

    sprintf_s(
        buffer,
        "[Door Init] points=%zu, links=%zu\n",
        model->GetDoorPoints().size(),
        _links.size()
    );

    OutputDebugStringA(buffer);
}

void DoorTransition::AddLink(
    const Matrix& triggerWorld,
    const Matrix& exitWorld,
    const std::string& bgmPath,
    bool destinationIndoor,
    const std::string& openSound,
    const std::string& requiredKey)
{
    DoorLink link;

    link.bgmPath = bgmPath;
    link.destinationIndoor = destinationIndoor;

    link.openSound = openSound;
    link.requiredKey = requiredKey;

    link.triggerPosition = XMVector3TransformCoord(
        Vec3::Zero,
        triggerWorld
    );

    link.exitPosition = XMVector3TransformCoord(
        Vec3::Zero,
        exitWorld
    );

    // 기존 SPAWN과 같은 전방 기준
    Vec3 forward = XMVector3TransformNormal(
        Vec3(0.f, 1.f, 0.f),
        exitWorld
    );

    forward.y = 0.f;

    if (forward.LengthSquared() > 0.000001f)
    {
        forward.Normalize();
        link.exitYaw = std::atan2(forward.x, forward.z);
    }

    _links.push_back(link);

}

bool DoorTransition::IsInside(
    const Vec3& footPosition,
    const DoorLink& link,
    float radius) const
{
    Vec3 difference =
        footPosition - link.triggerPosition;

    // 위층/아래층의 문이 작동하지 않도록 높이도 검사
    if (std::fabs(difference.y) > _heightTolerance)
        return false;

    difference.y = 0.f;

    return difference.LengthSquared() <= radius * radius;
}

void DoorTransition::Update()
{
    // 임시 진단: 최초 한 번만 출력
    static bool logged = false;

    if (!logged)
    {
        OutputDebugStringA("[Door] Update running\n");
        logged = true;
    }



    auto player = _player.lock();

    if (!player)
    {
        CUR_SCENE->SetFadeAlpha(0.f);
        return;
    }

    auto movement = player->GetCharacterMovement();

    if (!movement)
        return;

    float dt = TIME->GetDeltaTime();

    if (dt < 0.f)
        dt = 0.f;

    switch (_phase)
    {
    case Phase::Idle:
    {

        const Vec3 footPosition =
            movement->GetFootPosition();

        // 1. 문 사용 또는 잠금 확인 후에는
        // 감지 영역 밖으로 나가야 다시 시도 가능
        if (_waitUntilOutside)
        {
            bool insideAny = false;

            for (const auto& link : _links)
            {
                if (IsInside(
                    footPosition,
                    link,
                    _triggerRadius + 0.2f))
                {
                    insideAny = true;
                    break;
                }
            }

            if (!insideAny)
                _waitUntilOutside = false;

            return;
        }

        // 2. 공격 중이거나 이동이 정지된 상태면 사용 불가
        if (player->IsActionLocked() ||
            movement->IsMovementPaused())
        {
            return;
        }

        // 3. 접근한 문 검사
        for (const auto& link : _links)
        {
            if (!IsInside(
                footPosition,
                link,
                _triggerRadius))
            {
                continue;
            }

            // 4. 필요한 열쇠가 없으면 잠긴 소리만 재생
            if (!link.requiredKey.empty() &&
                !player->HasKey(link.requiredKey))
            {
                SoundManager::Get().PlaySFX(
                    link.lockedSound
                );

                // 계속 서 있어도 소리가 반복되지 않도록 설정
                _waitUntilOutside = true;
                return;
            }

            // 5. 사용 가능한 문이면 전환 시작
            _activeLink = link;

            player->Stop();
            movement->SetMovementPaused(true);

            SoundManager::Get().PlaySFX(
                link.openSound
            );

            _alpha = 0.f;
            CUR_SCENE->SetFadeAlpha(_alpha);

            _phase = Phase::FadeOut;
            break;
        }

        return;
    }

    case Phase::FadeOut:
    {
        _alpha += dt / _fadeDuration;

        if (_alpha >= 1.f)
        {
            _alpha = 1.f;
            _phase = Phase::Teleport;
        }

        CUR_SCENE->SetFadeAlpha(_alpha);
        return;
    }

    case Phase::Teleport:
    {
        // 직전 프레임에 완전히 검은 화면이 그려진 뒤 이동
        bool moved = movement->TeleportToGroundPoint(
            _activeLink.exitPosition,
            _activeLink.exitYaw
        );

        if (moved)
        {
            if (_resetCamera)
                _resetCamera();

            // 실내이면 눈 끄기, 실외이면 켜기
            CUR_SCENE->SetSnowEnabled(
                !_activeLink.destinationIndoor
            );

            if (!_activeLink.bgmPath.empty())
            {
                SoundManager::Get().PlayBGM(
                    _activeLink.bgmPath
                );
            }
        }

        CUR_SCENE->SetFadeAlpha(1.f);

        _waitUntilOutside = true;
        _phase = Phase::FadeIn;
        return;
    }

    case Phase::FadeIn:
    {
        _alpha -= dt / _fadeDuration;

        if (_alpha <= 0.f)
        {
            _alpha = 0.f;
            movement->SetMovementPaused(false);
            _phase = Phase::Idle;
        }

        CUR_SCENE->SetFadeAlpha(_alpha);
        return;
    }
    }
}