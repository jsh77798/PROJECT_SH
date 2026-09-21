#pragma once

#include "GameObject.h"
#include <string>

class Player;
class Shader;

class KeyItem : public GameObject
{
public:
    void Init(
        const std::string& keyID,
        const std::wstring& modelFile);
    void Update() override;

    bool CanPickup(Player& player);
    bool TryPickup(Player& player);

    void SetVisualHeight(float height);

private:
    std::string _keyID;
    shared_ptr<GameObject> _modelObject;

    float _pickupRadius = 2.0f;
    float _heightTolerance = 2.f;

    bool _collected = false;

    float _spinAngle = 0.f;
    float _spinSpeed = XMConvertToRadians(90.f); // 초당 90도
    float _tiltAngle = XMConvertToRadians(40.f); // 기울기
};