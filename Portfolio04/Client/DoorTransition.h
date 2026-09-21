#pragma once

#include "MonoBehaviour.h"
#include <functional>

class Player;
class Model;

class DoorTransition : public MonoBehaviour
{
public:
    void Init(
        shared_ptr<Player> player,
        shared_ptr<Model> model,
        const Matrix& mapWorld,
        std::function<void()> resetCamera);

    void Update() override;

private:
    enum class Phase
    {
        Idle,
        FadeOut,
        Teleport,
        FadeIn
    };

    struct DoorLink
    {
        Vec3 triggerPosition = Vec3::Zero;
        Vec3 exitPosition = Vec3::Zero;
        float exitYaw = 0.f;

        std::string bgmPath;
        bool destinationIndoor = false;

        std::string openSound = "DoorWood";
        std::string lockedSound = "DoorLocked";

        // 비어 있으면 열쇠가 필요 없는 문
        std::string requiredKey;
    };

    bool IsInside(
        const Vec3& footPosition,
        const DoorLink& link,
        float radius) const;

    void AddLink(
        const Matrix& triggerWorld,
        const Matrix& exitWorld,
        const std::string& bgmPath,
        bool destinationIndoor,
        const std::string& openSound,
        const std::string& requiredKey);

private:
    weak_ptr<Player> _player;

    vector<DoorLink> _links;
    std::function<void()> _resetCamera;

    Phase _phase = Phase::Idle;
    DoorLink _activeLink;

    float _alpha = 0.f;
    float _fadeDuration = 0.25f;

    // 게임 월드 단위
    float _triggerRadius = 0.5f;
    float _heightTolerance = 0.5f;

    // 출구에서 벗어날 때까지 재사용 불가
    bool _waitUntilOutside = false;
};