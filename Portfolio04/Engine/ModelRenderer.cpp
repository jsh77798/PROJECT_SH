#include "pch.h"
#include "ModelRenderer.h"
#include "Material.h"
#include "ModelMesh.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"

ModelRenderer::ModelRenderer(shared_ptr<Shader> shader)
	: Super(ComponentType::ModelRenderer), _shader(shader)
{

}

ModelRenderer::~ModelRenderer()
{

}

void ModelRenderer::SetModel(shared_ptr<Model> model)
{
	_model = model;

	const auto& materials = _model->GetMaterials();
	for (auto& material : materials)
	{
		material->SetShader(_shader);
	}
}

void ModelRenderer::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer)
{
	if (_model == nullptr)
		return;

	// 임시 진단
	if (_model->GetMeshes().size() == 295)
	{
		const uint32 meshCount =
			static_cast<uint32>(_model->GetMeshes().size());

		const uint32 boneCount =
			_model->GetBoneCount();

		const uint32 instanceCount =
			buffer->GetCount();

		char message[256];

		sprintf_s(
			message,
			"[Map Render] meshes=%u, nodes=%u, instances=%u, pass=%u\n",
			meshCount,
			boneCount,
			instanceCount,
			static_cast<unsigned int>(_pass)
		);

		OutputDebugStringA(message); // 여기에 중단점
	}

	// GlobalData
	_shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	// Light
	auto lightObj = SCENE->GetCurrentScene()->GetLight();
	if (lightObj)
		_shader->PushLightData(lightObj->GetLight()->GetLightDesc());

	//// Bones
	//BoneDesc boneDesc;
	//
	//const uint32 boneCount = _model->GetBoneCount();
	//for (uint32 i = 0; i < boneCount; i++)
	//{
	//	shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
	//	boneDesc.transforms[i] = bone->transform;
	//}
	//_shader->PushBoneData(boneDesc);


	// Bones
	BoneDesc boneDesc{};

	const uint32 boneCount = _model->GetBoneCount();

	// CPU 배열에 저장할 수 있는 행렬 개수
	const uint32 capacity = 
		sizeof(boneDesc.transforms) /
		sizeof(boneDesc.transforms[0]);

	// 배열 범위를 초과하면 렌더링 중단
	if (boneCount > capacity)
	{
		char message[256];

		sprintf_s(
			message,
			"[ModelRenderer] Node overflow: count=%u, capacity=%zu\n",
			boneCount,
			capacity
		);

		OutputDebugStringA(message);
		assert(false);
		return;
	}

	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone =
			_model->GetBoneByIndex(i);

		boneDesc.transforms[i] = bone->transform;
	}

	_shader->PushBoneData(boneDesc);


	const auto& meshes = _model->GetMeshes();
	for (auto& mesh : meshes)
	{
		if (mesh->material)
			mesh->material->Update();

		// BoneIndex
		_shader->GetScalar("BoneIndex")->SetInt(mesh->boneIndex);

		// IA
		mesh->vertexBuffer->PushData();
		mesh->indexBuffer->PushData();

		buffer->PushData();

		_shader->DrawIndexedInstanced(0, _pass, mesh->indexBuffer->GetCount(), buffer->GetCount());
	}
}

InstanceID ModelRenderer::GetInstanceID()
{
	return make_pair((uint64)_model.get(), (uint64)_shader.get());
}
