#include "pch.h"
#include "Map.h"
#include "AssetImporter.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "Transform.h"
#include "AABBBoxCollider.h"
#include "Scene.h"
//#include <cfloat>

void Map::Init(shared_ptr<Shader> shader, shared_ptr<Shader> debugShader)
{
    //////////////////////////// ResourceData ////////////////////////////

    auto model = make_shared<Model>();
    auto modelPath = ASSIMP->MeshImporter(L"Map/Map.fbx");
    model->ReadModel(modelPath);
    model->ReadMaterial(modelPath);
    //////////////////////////////////////////////////////////////////////


    auto transform = GetOrAddTransform();

    // 현재 사용 중인 맵 위치와 스케일로 설정
    const Vec3 mapPosition(0.f, 0.f, 0.f);
    const Vec3 mapScale(0.015f);

    transform->SetPosition(mapPosition);
    transform->SetRotation(Vec3(0.f, 0.f, 0.f));
    transform->SetScale(mapScale);

    auto renderer = make_shared<ModelRenderer>(shader);
    renderer->SetModel(model);
    renderer->SetPass(1);
    AddComponent(renderer);

    // 위 Transform 설정과 같은 행렬
    Matrix mapWorld =
        Matrix::CreateScale(mapScale) *
        Matrix::CreateTranslation(mapPosition);

    CreateWallColliders(model, debugShader, mapWorld);
}

void Map::CreateWallColliders(shared_ptr<Model> model, shared_ptr<Shader> debugShader, const Matrix& mapWorld)
{
    for (const auto& box : model->GetCollisionBoxes())
    {
        Vec3 worldMin(FLT_MAX, FLT_MAX, FLT_MAX);
        Vec3 worldMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

        // 모델 공간 박스의 8개 모서리에
        // 맵 전체의 위치·회전·스케일을 반영
        for (uint32 i = 0; i < 8; i++)
        {
            Vec3 corner(
                (i & 1) ? box.maxPosition.x : box.minPosition.x,
                (i & 2) ? box.maxPosition.y : box.minPosition.y,
                (i & 4) ? box.maxPosition.z : box.minPosition.z
            );

            Vec3 position =
                XMVector3TransformCoord(corner, mapWorld);

            if (position.x < worldMin.x) worldMin.x = position.x;
            if (position.y < worldMin.y) worldMin.y = position.y;
            if (position.z < worldMin.z) worldMin.z = position.z;

            if (position.x > worldMax.x) worldMax.x = position.x;
            if (position.y > worldMax.y) worldMax.y = position.y;
            if (position.z > worldMax.z) worldMax.z = position.z;
        }

        Vec3 center = (worldMin + worldMax) * 0.5f;
        Vec3 extents = (worldMax - worldMin) * 0.5f;

        auto wall = make_shared<GameObject>();
        wall->GetOrAddTransform()->SetPosition(center);

        auto collider =
            make_shared<AABBBoxCollider>(debugShader);

        collider->SetExtents(extents);
        wall->AddComponent(collider);

        collider->GetBoundingBox().Center = center;
        collider->GetBoundingBox().Extents = extents;

        CUR_SCENE->Add(wall);
    }
}
