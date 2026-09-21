#include "pch.h"
#include "KeyItem.h"

#include "Player.h"
#include "CharacterMovement.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Scene.h"
#include "AssetImporter.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "SoundManager.h"

#include <cmath>

void KeyItem::Init(
    const std::string& keyID,
    const std::wstring& modelFile)
{
    auto shader =
        make_shared<Shader>(L"23. RenderDemo.fx");

    _keyID = keyID;
    _collected = false;
    _spinAngle = 0.f;

    GetOrAddTransform();

    auto model = make_shared<Model>();
    auto modelPath = ASSIMP->MeshImporter(modelFile);

    model->ReadModel(modelPath);
    model->ReadMaterial(modelPath);

    // 회전시킬 모델 오브젝트 하나만 생성
    _modelObject = make_shared<GameObject>();

    auto transform = _modelObject->GetOrAddTransform();

    transform->SetScale(Vec3(0.006f));
    transform->SetLocalPosition(Vec3::Zero);
    transform->SetLocalRotation(
        Vec3(_tiltAngle, _spinAngle, 0.f)
    );

    auto renderer = make_shared<ModelRenderer>(shader);
    renderer->SetModel(model);
    renderer->SetPass(1);

    _modelObject->AddComponent(renderer);
    AddChild(_modelObject);
}

void KeyItem::Update()
{
    if (!_collected && _modelObject)
    {
        _spinAngle = std::fmod(
            _spinAngle + _spinSpeed * TIME->GetDeltaTime(),
            XM_2PI
        );

        _modelObject->GetTransform()->SetLocalRotation(
            Vec3(
                _tiltAngle,
                _spinAngle,
                0.f
            )
        );
    }

    GameObject::Update();
}

bool KeyItem::CanPickup(Player& player)
{
    if (_collected || _keyID.empty())
        return false;

    auto movement = player.GetCharacterMovement();

    if (!movement)
        return false;

    Vec3 difference =
        GetTransform()->GetPosition() -
        movement->GetFootPosition();

    // 다른 층의 열쇠를 획득하지 않도록 높이 검사
    if (std::fabs(difference.y) > _heightTolerance)
        return false;

    difference.y = 0.f;

    return difference.LengthSquared() <=
        _pickupRadius * _pickupRadius;
}

bool KeyItem::TryPickup(Player& player)
{
    if (!CanPickup(player))
        return false;

    // 중복 획득 방지
    _collected = true;

    player.AddKey(_keyID);

    // 획득 성공 시 한 번 재생
    SoundManager::Get().PlaySFX("ItemPickup");

    OutputDebugStringA(
        ("[Item] Key acquired: " + _keyID + "\n").c_str()
    );

    CUR_SCENE->Remove(shared_from_this());

    return true;
}

void KeyItem::SetVisualHeight(float height)
{
    if (!_modelObject)
        return;

    auto transform = _modelObject->GetTransform();

    Vec3 position = transform->GetLocalPosition();
    position.y = height;

    transform->SetLocalPosition(position);
}
