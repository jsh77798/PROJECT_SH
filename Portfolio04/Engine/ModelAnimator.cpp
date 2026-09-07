#include "pch.h"
#include "ModelAnimator.h"
#include "Material.h"
#include "ModelMesh.h"
#include "Model.h"
#include "ModelAnimation.h"
#include "Camera.h"
#include "Light.h"

ModelAnimator::ModelAnimator(shared_ptr<Shader> shader)
	: Super(ComponentType::Animator), _shader(shader)
{
	// TEST
	_tweenDesc.next.animIndex = -1; //rand() % 3;
	_tweenDesc.tweenSumTime += rand() % 100;
}

ModelAnimator::~ModelAnimator()
{

}

void ModelAnimator::SetModel(shared_ptr<Model> model)
{
	//_model = model;
	//
	//const auto& materials = _model->GetMaterials();
	//for (auto& material : materials)
	//{
	//	material->SetShader(_shader);
	//}

	_model = model;

	if (_model == nullptr)
		return;

	_currentBoneTransforms.resize(
		_model->GetBoneCount(),
		Matrix::Identity);

	const auto& materials = _model->GetMaterials();

	for (auto& material : materials)
		material->SetShader(_shader);
}

Matrix ModelAnimator::GetBoneTransform(const wstring& boneName)
{
	//if (_model == nullptr)
	//	return Matrix::Identity;
	//
	//shared_ptr<ModelBone> bone =
	//	_model->GetBoneByName(boneName);
	//
	//if (bone == nullptr)
	//	return Matrix::Identity;
	//
	//return _currentBoneTransforms[bone->index];

	if (_model == nullptr)
		return Matrix::Identity;
	
	shared_ptr<ModelBone> bone =
		_model->GetBoneByName(boneName);

	if (bone == nullptr)
	{
		cout << "Bone Not Found!" << endl;
		return Matrix::Identity;
	}
	
	if (bone == nullptr)
		return Matrix::Identity;
	
	cout << "Bone Index : "
		<< bone->index
		<< endl;
	
	cout << "Bone Transform Count : "
		<< _currentBoneTransforms.size()
		<< endl;
	
	if (bone->index < 0 ||
		bone->index >= _currentBoneTransforms.size())
	{
		cout << "Invalid Bone Index!" << endl;
		return Matrix::Identity;
	}
	
	return _currentBoneTransforms[bone->index];
}

void ModelAnimator::Update()
{
	UpdateTweenData();

	UpdateBoneTransforms();

	UpdateCurrentBoneTransforms();
}

void ModelAnimator::UpdateTweenData()
{
	TweenDesc& desc = _tweenDesc;

	desc.curr.sumTime += DT;
	// 현재 애니메이션
	{
		shared_ptr<ModelAnimation> currentAnim = _model->GetAnimationByIndex(desc.curr.animIndex);
		if (currentAnim)
		{
			float timePerFrame = 1 / (currentAnim->frameRate * desc.curr.speed);
			if (desc.curr.sumTime >= timePerFrame)
			{
				//desc.curr.sumTime = 0;
				//desc.curr.currFrame = (desc.curr.currFrame + 1) % currentAnim->frameCount;
				//desc.curr.nextFrame = (desc.curr.currFrame + 1) % currentAnim->frameCount;

				desc.curr.sumTime = 0;

				if (desc.curr.currFrame >= currentAnim->frameCount - 1)
				{
					if (_loop)
					{
						desc.curr.currFrame = 0;
						desc.curr.nextFrame =
							currentAnim->frameCount > 1 ? 1 : 0;
					}
					else
					{
						_isAnimationFinished = true;

						desc.curr.currFrame =
							currentAnim->frameCount - 1;

						desc.curr.nextFrame =
							desc.curr.currFrame;
					}
				}
				else
				{
					desc.curr.currFrame++;

					desc.curr.nextFrame =
						(desc.curr.currFrame + 1) %
						currentAnim->frameCount;
				}
			}

			desc.curr.ratio = (desc.curr.sumTime / timePerFrame);
		}
	}

	 //다음 애니메이션이 예약 되어 있다면
	if (desc.next.animIndex >= 0)
	{
		desc.tweenSumTime += DT;
		desc.tweenRatio = desc.tweenSumTime / desc.tweenDuration;
		
		if (desc.tweenRatio >= 1.f)
		{
			// 애니메이션 교체 성공
			desc.curr = desc.next;
			desc.ClearNextAnim();
		}
		else
		{
			// 교체중
			shared_ptr<ModelAnimation> nextAnim = _model->GetAnimationByIndex(desc.next.animIndex);
			desc.next.sumTime += DT;
		
			float timePerFrame = 1.f / (nextAnim->frameRate * desc.next.speed);
		
			if (desc.next.sumTime >= timePerFrame)
			{
				desc.next.sumTime = 0;
		
				desc.next.currFrame = (desc.next.currFrame + 1) % nextAnim->frameCount;
				desc.next.nextFrame = (desc.next.currFrame + 1) % nextAnim->frameCount;
			}
		
			desc.next.ratio = desc.next.sumTime / timePerFrame;
		}

	}
}

void ModelAnimator::UpdateBoneTransforms()
{
	if (_model == nullptr)
		return;

	if (_tweenDesc.curr.animIndex < 0)
		return;

	shared_ptr<ModelAnimation> animation =
		_model->GetAnimationByIndex(
			_tweenDesc.curr.animIndex);

	if (animation == nullptr)
		return;

	vector<Matrix> localTransforms(
		_model->GetBoneCount(),
		Matrix::Identity);

	for (uint32 i = 0;
		i < _model->GetBoneCount();
		i++)
	{
		shared_ptr<ModelBone> bone =
			_model->GetBoneByIndex(i);

		Matrix matAnimation =
			Matrix::Identity;

		shared_ptr<ModelKeyframe> frame =
			animation->GetKeyframe(bone->name);

		if (frame != nullptr)
		{
			uint32 frameIndex =
				_tweenDesc.curr.currFrame;

			ModelKeyframeData& data =
				frame->transforms[frameIndex];

			Matrix S =
				Matrix::CreateScale(
					data.scale.x,
					data.scale.y,
					data.scale.z);

			Matrix R =
				Matrix::CreateFromQuaternion(
					data.rotation);

			Matrix T =
				Matrix::CreateTranslation(
					data.translation.x,
					data.translation.y,
					data.translation.z);

			matAnimation = S * R * T;
		}

		int32 parentIndex =
			bone->parentIndex;

		if (parentIndex >= 0)
		{
			_currentBoneTransforms[i] =
				matAnimation *
				_currentBoneTransforms[parentIndex];
		}
		else
		{
			_currentBoneTransforms[i] =
				matAnimation;
		}
	}
}

void ModelAnimator::UpdateCurrentBoneTransforms()
{
	if (_model == nullptr)
		return;

	TweenDesc& desc = _tweenDesc;

	shared_ptr<ModelAnimation> animation =
		_model->GetAnimationByIndex(desc.curr.animIndex);

	if (animation == nullptr)
		return;

	uint32 boneCount = _model->GetBoneCount();

	_currentBoneTransforms.resize(
		boneCount,
		Matrix::Identity);

	uint32 frameIndex = desc.curr.currFrame;

	if (frameIndex >= animation->frameCount)
		frameIndex = animation->frameCount - 1;

	for (uint32 b = 0; b < boneCount; b++)
	{
		shared_ptr<ModelBone> bone =
			_model->GetBoneByIndex(b);

		if (bone == nullptr)
			continue;

		Matrix matAnimation =
			Matrix::Identity;

		shared_ptr<ModelKeyframe> frame =
			animation->GetKeyframe(bone->name);

		if (frame != nullptr)
		{
			if (frameIndex >= frame->transforms.size())
				continue;

			ModelKeyframeData& data =
				frame->transforms[frameIndex];

			Matrix S;
			Matrix R;
			Matrix T;

			S = Matrix::CreateScale(
				data.scale.x,
				data.scale.y,
				data.scale.z);

			R = Matrix::CreateFromQuaternion(
				data.rotation);

			T = Matrix::CreateTranslation(
				data.translation.x,
				data.translation.y,
				data.translation.z);

			matAnimation = S * R * T;
		}

		Matrix parentMatrix =
			Matrix::Identity;

		if (bone->parentIndex >= 0 &&
			bone->parentIndex < boneCount)
		{
			parentMatrix =
				_currentBoneTransforms[
					bone->parentIndex];
		}

		_currentBoneTransforms[b] =
			matAnimation * parentMatrix;
	}
}

void ModelAnimator::RenderInstancing(shared_ptr<class InstancingBuffer>& buffer)
{
	if (_model == nullptr)
		return;
	if (_texture == nullptr)
		CreateTexture();

	// GlobalData
	_shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	// Light
	auto lightObj = SCENE->GetCurrentScene()->GetLight();
	if (lightObj)
		_shader->PushLightData(lightObj->GetLight()->GetLightDesc());

	// SRV를 통해 정보 전달
	_shader->GetSRV("TransformMap")->SetResource(_srv.Get());

	// Bones
	BoneDesc boneDesc;

	const uint32 boneCount = _model->GetBoneCount();
	for (uint32 i = 0; i < boneCount; i++)
	{
		shared_ptr<ModelBone> bone = _model->GetBoneByIndex(i);
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

		mesh->vertexBuffer->PushData();
		mesh->indexBuffer->PushData();

		buffer->PushData();

		_shader->DrawIndexedInstanced(0, _pass, mesh->indexBuffer->GetCount(), buffer->GetCount());
	}
}

InstanceID ModelAnimator::GetInstanceID()
{
	return make_pair((uint64)_model.get(), (uint64)_shader.get());
}

void ModelAnimator::Play(string animName)
{
	auto iter = _animDataMap.find(animName);
	if (iter == _animDataMap.end())	
		return;

	AnimData animData = _animDataMap[animName];
	
	if (_tweenDesc.curr.animIndex == animData.animIndex)
		return;

	_tweenDesc.next.animIndex = animData.animIndex;
	_loop = animData.animLoop;

	_tweenDesc.next.currFrame = 0;
	_tweenDesc.next.nextFrame = 1;
	_tweenDesc.next.sumTime = 0.f;
	_tweenDesc.next.ratio = 0.f;
	_tweenDesc.next.speed = animData.speed;

	_tweenDesc.tweenSumTime = 0.f;
	_tweenDesc.tweenRatio = 0.f;

	_tweenDesc.tweenDuration = 0.2f;
}

bool ModelAnimator::IsAnimationFinished()
{
	return _isAnimationFinished;;
}

void ModelAnimator::CreateTexture()
{
	if (_model->GetAnimationCount() == 0)
		return;

	_animTransforms.resize(_model->GetAnimationCount());
	for (uint32 i = 0; i < _model->GetAnimationCount(); i++)
		CreateAnimationTransform(i);

	// Creature Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = MAX_MODEL_TRANSFORMS * 4;
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = _model->GetAnimationCount();
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 16바이트
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		const uint32 dataSize = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
		const uint32 pageSize = dataSize * MAX_MODEL_KEYFRAMES;
		void* mallocPtr = ::malloc(pageSize * _model->GetAnimationCount());

		// 파편화된 데이터를 조립한다.
		for (uint32 c = 0; c < _model->GetAnimationCount(); c++)
		{
			uint32 startOffset = c * pageSize;

			BYTE* pageStartPtr = reinterpret_cast<BYTE*>(mallocPtr) + startOffset;

			for (uint32 f = 0; f < MAX_MODEL_KEYFRAMES; f++)
			{
				void* ptr = pageStartPtr + dataSize * f;
				::memcpy(ptr, _animTransforms[c].transforms[f].data(), dataSize);
			}
		}

		// 리소스 만들기
		vector<D3D11_SUBRESOURCE_DATA> subResources(_model->GetAnimationCount());

		for (uint32 c = 0; c < _model->GetAnimationCount(); c++)
		{
			void* ptr = (BYTE*)mallocPtr + c * pageSize;
			subResources[c].pSysMem = ptr;
			subResources[c].SysMemPitch = dataSize;
			subResources[c].SysMemSlicePitch = pageSize;
		}

		HRESULT hr = DEVICE->CreateTexture2D(&desc, subResources.data(), _texture.GetAddressOf());
		CHECK(hr);

		::free(mallocPtr);
	}

	// Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = _model->GetAnimationCount();

		HRESULT hr = DEVICE->CreateShaderResourceView(_texture.Get(), &desc, _srv.GetAddressOf());
		CHECK(hr);
	}
}

void ModelAnimator::CreateAnimationTransform(uint32 index)
{
	vector<Matrix> tempAnimBoneTransforms(MAX_MODEL_TRANSFORMS, Matrix::Identity);

	shared_ptr<ModelAnimation> animation = _model->GetAnimationByIndex(index);

	for (uint32 f = 0; f < animation->frameCount; f++)
	{
		for (uint32 b = 0; b < _model->GetBoneCount(); b++)
		{
			shared_ptr<ModelBone> bone = _model->GetBoneByIndex(b);

			Matrix matAnimation;

			shared_ptr<ModelKeyframe> frame = animation->GetKeyframe(bone->name);
			if (frame != nullptr)
			{
				ModelKeyframeData& data = frame->transforms[f];

				Matrix S, R, T;
				S = Matrix::CreateScale(data.scale.x, data.scale.y, data.scale.z);
				R = Matrix::CreateFromQuaternion(data.rotation);
				T = Matrix::CreateTranslation(data.translation.x, data.translation.y, data.translation.z);

				matAnimation = S * R * T;
			}
			else
			{
				matAnimation = Matrix::Identity;
			}

			// [ !!!!!!! ]
			Matrix toRootMatrix = bone->transform;
			Matrix invGlobal = toRootMatrix.Invert();

			int32 parentIndex = bone->parentIndex;

			Matrix matParent = Matrix::Identity;
			if (parentIndex >= 0)
				matParent = tempAnimBoneTransforms[parentIndex];

			tempAnimBoneTransforms[b] = matAnimation * matParent;

			// 결론
			_animTransforms[index].transforms[f][b] = invGlobal * tempAnimBoneTransforms[b];
		}
	}
}
