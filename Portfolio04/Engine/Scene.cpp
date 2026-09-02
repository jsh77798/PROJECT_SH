#include "pch.h"
#include "Scene.h"
#include "GameObject.h"
#include "BaseCollider.h"
#include "AABBBoxCollider.h"
#include "SphereCollider.h"
#include "Camera.h"
#include "Terrain.h"
#include "Button.h"

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
	unordered_set<shared_ptr<GameObject>> objects = _objects;

	for (shared_ptr<GameObject> object : objects)
	{
		object->LateUpdate();
	}

	CheckCollision();
}

void Scene::Render()
{
	for (auto& camera : _cameras)
	{
		camera->GetCamera()->SortGameObject();
		camera->GetCamera()->Render_Forward();
	}

	// Collider Debug Render
	for (auto& object : _objects)
	{
		auto collider = object->GetCollider();

		if (collider == nullptr)
			continue;

		collider->DebugRender();
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

bool Scene::RayCast(Ray& ray, shared_ptr<BaseCollider>& ignoreCollider, OUT shared_ptr<BaseCollider>& hitCollider, OUT float& distance)
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
