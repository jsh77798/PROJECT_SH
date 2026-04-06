#include "pch.h"
#include "AnimInstancingDemo.h"
#include "GeometryHelper.h"
#include "Camera.h"
#include "GameObject.h"
#include "Light.h"
#include "CameraScript.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"
#include "Mesh.h"
#include "Transform.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

void AnimInstancingDemo::Init()
{
	RESOURCES->Init();
	_shader = make_shared<Shader>(L"22. AnimInstancingDemo.fx");

	// Camera
	{
		auto camera = make_shared<GameObject>();
		camera->GetOrAddTransform()->SetPosition(Vec3{ 0.f, 0.f, -5.f });
		camera->AddComponent(make_shared<Camera>());
		camera->AddComponent(make_shared<CameraScript>());
		camera->GetCamera()->SetCullingMaskLayerOnOff(Layer_UI, true);
		CUR_SCENE->Add(camera);
	}

	// Light
	{
		auto light = make_shared<GameObject>();
		light->AddComponent(make_shared<Light>());
		LightDesc lightDesc;
		lightDesc.ambient = Vec4(0.4f);
		lightDesc.diffuse = Vec4(1.f);
		lightDesc.specular = Vec4(0.1f);
		lightDesc.direction = Vec3(1.f, 0.f, 1.f);
		light->GetLight()->SetLightDesc(lightDesc);
		CUR_SCENE->Add(light);
	}

	shared_ptr<class Model> m1 = make_shared<Model>();
	m1->ReadModel(L"Kachujin/Kachujin");
	m1->ReadMaterial(L"Kachujin/Kachujin");
	m1->ReadAnimation(L"Kachujin/Idle");
	m1->ReadAnimation(L"Kachujin/Run");
	m1->ReadAnimation(L"Kachujin/Slash");

	for (int32 i = 0; i < 500; i++)
	{
		auto obj = make_shared<GameObject>();
		obj->GetOrAddTransform()->SetPosition(Vec3(rand() % 100, 0, rand() % 100));
		obj->GetOrAddTransform()->SetScale(Vec3(0.01f));
		obj->AddComponent(make_shared<ModelAnimator>(_shader));
		{
			obj->GetModelAnimator()->SetModel(m1);
		}
		_objs.push_back(obj);
	}

	//RENDER->Init(_shader);
}

void AnimInstancingDemo::Update()
{
	//_camera->Update();
	//RENDER->Update();

	//{
	//	LightDesc lightDesc;
	//	lightDesc.ambient = Vec4(0.4f);
	//	lightDesc.diffuse = Vec4(1.f);
	//	lightDesc.specular = Vec4(0.1f);
	//	lightDesc.direction = Vec3(1.f, 0.f, 1.f);
	//	//RENDER->PushLightData(lightDesc);
	//}

	// INSTANCING
	INSTANCING->Render(_objs);
}

void AnimInstancingDemo::Render()
{

}