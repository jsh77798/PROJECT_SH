#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "BaseCollider.h"
#include "AABBBoxCollider.h"
#include "SphereCollider.h"
#include "Camera.h"
#include "Terrain.h"
#include "Button.h"
#include "SnowBillboard.h"
#include "Skybox.h"

void Scene::Start()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->Start();
	}
}

void Scene::Update()
{
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->Update();
	}

	PickUI();
}

void Scene::LateUpdate()
{
	//unordered_set<shared_ptr<GameObject>> objects = _objects;
	//
	//for (shared_ptr<GameObject> object : objects)
	//{
	//	object->LateUpdate();
	//}
	//
	//CheckCollision();

	unordered_set<shared_ptr<GameObject>> objects = _objects;

	// 1. 실제 이동 처리
	for (auto& object : objects)
	{
		object->LateUpdate();
	}

	// 2. 충돌 처리
	CheckCollision();

	// 3. 이동·충돌 이후 처리
	// CameraScript의 추적도 여기서 실행됨
	for (auto& object : objects)
	{
		object->PostLateUpdate();
	}

	// 4. 모든 추적 처리가 끝난 뒤 카메라 행렬 갱신
	auto updateCameraMatrices =
		[](auto&& self, const shared_ptr<GameObject>& object) -> void
		{
			if (auto camera = object->GetCamera())
			{
				camera->UpdateMatrix();
			}

			for (auto& child : object->GetChildren())
			{
				self(self, child);
			}
		};

	for (auto& object : objects)
	{
		updateCameraMatrices(updateCameraMatrices, object);
	}
}

void Scene::Render()
{
	//for (auto& camera : _cameras)
	//{
	//	camera->GetCamera()->SortGameObject();
	//	camera->GetCamera()->Render_Forward();
	//}
	//
	//// Collider Debug Render
	//for (auto& object : _objects)
	//{
	//	auto collider = object->GetCollider();
	//
	//	if (collider == nullptr)
	//		continue;
	//
	//	collider->DebugRender();
	//}

	// 자식 오브젝트의 눈 컴포넌트까지 렌더링
	auto renderSnow =
		[](auto&& self,
			const shared_ptr<GameObject>& object,
			const Vec3& cameraPosition) -> void
		{
			if (auto snow = object->GetSnowBillboard())
			{
				snow->Render(cameraPosition);
			}

			for (auto& child : object->GetChildren())
			{
				self(self, child, cameraPosition);
			}
		};

	// 1. 월드 카메라
	for (auto& cameraObject : _cameras)
	{
		auto camera = cameraObject->GetCamera();

		if (camera->GetProjectionType() !=
			ProjectionType::Perspective)
		{
			continue;
		}

		// 맵·캐릭터 렌더링 및 현재 카메라 행렬 설정
		camera->SortGameObject();
		camera->Render_Forward();

		// 추가: 하늘 렌더링
		if (_skybox)
		{
			_skybox->Render();
		}

		// 눈 렌더링
		Vec3 cameraPosition =
			cameraObject->GetTransform()->GetPosition();

		for (auto& object : _objects)
		{
			renderSnow(renderSnow, object, cameraPosition);
		}

		// 콜리전도 월드 카메라 행렬로 렌더링
		for (auto& object : _objects)
		{
			auto collider = object->GetCollider();

			if (collider)
			{
				collider->DebugRender();
			}
		}
	}

	// 2. UI 카메라 ? 마지막에 렌더링
	for (auto& cameraObject : _cameras)
	{
		auto camera = cameraObject->GetCamera();

		if (camera->GetProjectionType() !=
			ProjectionType::Orthographic)
		{
			continue;
		}

		camera->SortGameObject();
		camera->Render_Forward();
	}
}

void Scene::Add(shared_ptr<GameObject> object)
{
	_objects.insert(object);

	if (object->GetCamera() != nullptr)
	{
		_cameras.insert(object);
	}

	if (object->GetLight() != nullptr)
	{
		_lights.insert(object);
	}

	object->Awake();

	object->Start();
}

void Scene::Remove(shared_ptr<GameObject> object)
{
	_objects.erase(object);

	_cameras.erase(object);

	_lights.erase(object);
}

std::shared_ptr<GameObject> Scene::GetMainCamera()
{
	for (auto& camera : _cameras)
	{
		if (camera->GetCamera()->GetProjectionType() == ProjectionType::Perspective)
			return camera;
	}

	return nullptr;
}

std::shared_ptr<GameObject> Scene::GetUICamera()
{
	for (auto& camera : _cameras)
	{
		if (camera->GetCamera()->GetProjectionType() == ProjectionType::Orthographic)
			return camera;
	}

	return nullptr;
}

bool Scene::RayCast(Ray& ray, shared_ptr<BaseCollider>& ignoreCollider, OUT shared_ptr<BaseCollider>& hitCollider, OUT float& distance, bool cameraOnly)
{
	hitCollider = nullptr;
	distance = FLT_MAX;

	for (shared_ptr<GameObject> object : _objects)
	{
		shared_ptr<BaseCollider> collider =
			object->GetCollider();

		if (collider == nullptr)
			continue;

		if (collider == ignoreCollider)
			continue;

		if (cameraOnly && !collider->BlocksCamera())
			continue;

		float hitDistance = 0.f;

		if (collider->Intersects(
			ray,
			hitDistance) == false)
		{
			continue;
		}

		if (hitDistance < distance)
		{
			distance = hitDistance;
			hitCollider = collider;
		}
	}

	return hitCollider != nullptr;
}

void Scene::PickUI()
{
	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON) == false)
		return;

	if (GetUICamera() == nullptr)
		return;

	POINT screenPt = INPUT->GetMousePos();

	shared_ptr<Camera> camera = GetUICamera()->GetCamera();

	const auto gameObjects = GetObjects();

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetButton() == nullptr)
			continue;

		if (gameObject->GetButton()->Picked(screenPt))
			gameObject->GetButton()->InvokeOnClicked();
	}
}

std::shared_ptr<class GameObject> Scene::Pick(int32 screenX, int32 screenY)
{
	shared_ptr<Camera> camera = GetMainCamera()->GetCamera();

	float width = GRAPHICS->GetViewport().GetWidth();
	float height = GRAPHICS->GetViewport().GetHeight();

	Matrix projectionMatrix = camera->GetProjectionMatrix();

	float viewX = (+2.0f * screenX / width - 1.0f) / projectionMatrix(0, 0);
	float viewY = (-2.0f * screenY / height + 1.0f) / projectionMatrix(1, 1);

	Matrix viewMatrix = camera->GetViewMatrix();
	Matrix viewMatrixInv = viewMatrix.Invert();

	const auto& gameObjects = GetObjects();

	float minDistance = FLT_MAX;
	shared_ptr<GameObject> picked;

	for (auto& gameObject : gameObjects)
	{
		if (camera->IsCulled(gameObject->GetLayerIndex()))
			continue;
		if (gameObject->GetCollider() == nullptr)
			continue;

		// ViewSpace에서의 Ray 정의
		Vec4 rayOrigin = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
		Vec4 rayDir = Vec4(viewX, viewY, 1.0f, 0.0f);

		// WorldSpace에서의 Ray 정의
		Vec3 worldRayOrigin = XMVector3TransformCoord(rayOrigin, viewMatrixInv);
		Vec3 worldRayDir = XMVector3TransformNormal(rayDir, viewMatrixInv);
		worldRayDir.Normalize();

		// WorldSpace에서 연산
		Ray ray = Ray(worldRayOrigin, worldRayDir);

		float distance = 0.f;
		if (gameObject->GetCollider()->Intersects(ray, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	for (auto& gameObject : gameObjects)
	{
		if (gameObject->GetTerrain() == nullptr)
			continue;

		Vec3 pickPos;
		float distance = 0.f;
		if (gameObject->GetTerrain()->Pick(screenX, screenY, OUT pickPos, OUT distance) == false)
			continue;

		if (distance < minDistance)
		{
			minDistance = distance;
			picked = gameObject;
		}
	}

	return picked;
}

void Scene::CheckCollision()
{
	vector<shared_ptr<BaseCollider>> colliders;

	for (shared_ptr<GameObject> object : _objects)
	{
		if (object->GetCollider() == nullptr)
			continue;

		colliders.push_back(object->GetCollider());
	}

	// BruteForce
	for (int32 i = 0; i < colliders.size(); i++)
	{
		for (int32 j = i + 1; j < colliders.size(); j++)
		{
			shared_ptr<BaseCollider>& other = colliders[j];
			if (colliders[i]->Intersects(other))
			{
				int a = 3;
			}
		}
	}
}

bool Scene::CheckCollision(shared_ptr<BaseCollider>& collider, OUT Vec3& normal)
{
	if (collider == nullptr)
		return false;

	for (shared_ptr<GameObject> object : _objects)
	{
		shared_ptr<BaseCollider> other =
			object->GetCollider();

		if (other == nullptr)
			continue;

		if (other == collider)
			continue;

		if (collider->Intersects(other) == false)
			continue;

		// Sphere
		if (collider->GetColliderType() == ColliderType::Sphere)
		{
			auto sphere =
				dynamic_pointer_cast<SphereCollider>(collider);

			if (sphere->GetCollisionNormal(
				other,
				normal))
			{
				return true;
			}
		}

		// AABB
		else if (collider->GetColliderType() == ColliderType::AABB)
		{
			auto aabb =
				dynamic_pointer_cast<AABBBoxCollider>(collider);

			if (aabb->GetCollisionNormal(
				other,
				normal))
			{
				return true;
			}
		}
	}

	return false;
}
