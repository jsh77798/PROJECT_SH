#pragma once
#include "BaseCollider.h"

class Shader;
class Skybox;

class Scene
{
public:
	virtual void Start();
	virtual void Update();
	virtual void LateUpdate();

	virtual void Render();

	virtual void Add(shared_ptr<GameObject> object);
	virtual void Remove(shared_ptr<GameObject> object);

	unordered_set<shared_ptr<GameObject>>& GetObjects() { return _objects; }
	shared_ptr<GameObject> GetMainCamera();
	shared_ptr<GameObject> GetUICamera();
	shared_ptr<GameObject> GetLight() { return _lights.empty() ? nullptr : *_lights.begin(); }

	void SetSkybox(shared_ptr<Skybox> skybox)
	{
		_skybox = skybox;
	}

	void SetFadeAlpha(float alpha)
	{
		_fadeAlpha =
			alpha < 0.f ? 0.f :
			alpha > 1.f ? 1.f : alpha;
	}

	bool RayCast(
		Ray& ray,
		shared_ptr<BaseCollider>& ignoreCollider,
		OUT shared_ptr<BaseCollider>& hitCollider,
		OUT float& distance,
		bool cameraOnly = false);

	bool RayCastFiltered(
		Ray& ray,
		float maxDistance,
		const std::function<bool(
			const shared_ptr<GameObject>&)>& shouldIgnore,
		OUT shared_ptr<BaseCollider>& hitCollider,
		OUT float& distance);

	void PickUI();
	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

	void CheckCollision();
	bool CheckCollision(shared_ptr<BaseCollider>& collider, OUT Vec3& normal);

private:
	void RenderFade();

private:
	unordered_set<shared_ptr<GameObject>> _objects;
	// Cache Camera
	unordered_set<shared_ptr<GameObject>> _cameras;
	// Cache Light
	unordered_set<shared_ptr<GameObject>> _lights;
	shared_ptr<Skybox> _skybox;
	float _fadeAlpha = 0.f;
	shared_ptr<Shader> _fadeShader;
};

