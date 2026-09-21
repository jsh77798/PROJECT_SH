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
	//_tweenDesc.next.animIndex = -1; //rand() % 3;
	//_tweenDesc.tweenSumTime += rand() % 100;
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

	//UpdateBoneTransforms();

	UpdateCurrentBoneTransforms();
}

void ModelAnimator::UpdateTweenData()
{
	//TweenDesc& desc = _tweenDesc;
	//
	//desc.curr.sumTime += DT;
	//// 현재 애니메이션
	//{
	//	shared_ptr<ModelAnimation> currentAnim = _model->GetAnimationByIndex(desc.curr.animIndex);
	//	if (currentAnim)
	//	{
	//		float timePerFrame = 1 / (currentAnim->frameRate * desc.curr.speed);
	//		if (desc.curr.sumTime >= timePerFrame)
	//		{
	//			//desc.curr.sumTime = 0;
	//			//desc.curr.currFrame = (desc.curr.currFrame + 1) % currentAnim->frameCount;
	//			//desc.curr.nextFrame = (desc.curr.currFrame + 1) % currentAnim->frameCount;
	//
	//			desc.curr.sumTime -= timePerFrame;
	//
	//			if (desc.curr.currFrame >= currentAnim->frameCount - 1)
	//			{
	//				if (_loop)
	//				{
	//					desc.curr.currFrame = 0;
	//					desc.curr.nextFrame =
	//						currentAnim->frameCount > 1 ? 1 : 0;
	//				}
	//				else
	//				{
	//					_isAnimationFinished = true;
	//
	//					desc.curr.currFrame =
	//						currentAnim->frameCount - 1;
	//
	//					desc.curr.nextFrame =
	//						desc.curr.currFrame;
	//				}
	//			}
	//			else
	//			{
	//				//desc.curr.currFrame++;
	//				//
	//				//desc.curr.nextFrame =
	//				//	(desc.curr.currFrame + 1) %
	//				//	currentAnim->frameCount;
	//
	//				desc.curr.currFrame++;
	//
	//				if (desc.curr.currFrame + 1 < currentAnim->frameCount)
	//				{
	//					desc.curr.nextFrame = desc.curr.currFrame + 1;
	//				}
	//				else
	//				{
	//					// 반복이면 첫 프레임으로 연결
	//					// 반복하지 않으면 마지막 프레임 유지
	//					desc.curr.nextFrame =
	//						_loop ? 0 : desc.curr.currFrame;
	//				}
	//			}
	//		}
	//
	//		desc.curr.ratio = (desc.curr.sumTime / timePerFrame);
	//	}
	//}
	//
	// //다음 애니메이션이 예약 되어 있다면
	//if (desc.next.animIndex >= 0)
	//{
	//	desc.tweenSumTime += DT;
	//	desc.tweenRatio = desc.tweenSumTime / desc.tweenDuration;
	//	
	//	if (desc.tweenRatio >= 1.f)
	//	{
	//		// 애니메이션 교체 성공
	//		desc.curr = desc.next;
	//		desc.ClearNextAnim();
	//
	//		_isAnimationFinished = false;
	//	}
	//	else
	//	{
	//		// 교체중
	//		shared_ptr<ModelAnimation> nextAnim = _model->GetAnimationByIndex(desc.next.animIndex);
	//		desc.next.sumTime += DT;
	//	
	//		float timePerFrame = 1.f / (nextAnim->frameRate * desc.next.speed);
	//	
	//		if (desc.next.sumTime >= timePerFrame)
	//		{
	//			desc.next.sumTime -= timePerFrame;
	//	
	//			desc.next.currFrame = (desc.next.currFrame + 1) % nextAnim->frameCount;
	//			desc.next.nextFrame = (desc.next.currFrame + 1) % nextAnim->frameCount;
	//		}
	//	
	//		desc.next.ratio = desc.next.sumTime / timePerFrame;
	//	}
	//
	//}

	if (!_model)
		return;

	TweenDesc& desc = _tweenDesc;

	const float dt = DT > 0.f ? DT : 0.f;

	// 현재/다음 애니메이션에 동일한 프레임 진행 규칙 적용
	auto advance = [&](auto& frame, bool loop) -> bool
		{
			if (frame.animIndex < 0)
				return false;

			auto animation =
				_model->GetAnimationByIndex(frame.animIndex);

			if (!animation || animation->frameCount == 0)
				return false;

			const uint32 frameCount = animation->frameCount;
			const uint32 lastFrame = frameCount - 1;

			if (frameCount == 1)
			{
				frame.currFrame = 0;
				frame.nextFrame = 0;
				frame.sumTime = 0.f;
				frame.ratio = 0.f;

				return !loop;
			}

			// 비반복 애니메이션은 마지막 자세 유지
			if (!loop && frame.currFrame >= lastFrame)
			{
				frame.currFrame = lastFrame;
				frame.nextFrame = lastFrame;
				frame.sumTime = 0.f;
				frame.ratio = 0.f;

				return true;
			}

			if (animation->frameRate <= 0.f || frame.speed <= 0.f)
				return false;

			const float timePerFrame =
				1.f / (animation->frameRate * frame.speed);

			frame.sumTime += dt;

			// 큰 DT에서도 필요한 만큼 프레임 진행
			while (frame.sumTime >= timePerFrame)
			{
				frame.sumTime -= timePerFrame;

				if (loop)
				{
					frame.currFrame =
						(frame.currFrame + 1) % frameCount;
				}
				else
				{
					++frame.currFrame;

					if (frame.currFrame >= lastFrame)
					{
						frame.currFrame = lastFrame;
						frame.nextFrame = lastFrame;
						frame.sumTime = 0.f;
						frame.ratio = 0.f;

						return true;
					}
				}
			}

			frame.nextFrame = loop
				? (frame.currFrame + 1) % frameCount
				: frame.currFrame + 1;

			frame.ratio = frame.sumTime / timePerFrame;

			return false;
		};

	const bool currentFinished =
		advance(desc.curr, _loop);

	if (desc.next.animIndex < 0)
	{
		_isAnimationFinished = currentFinished;
		return;
	}

	// 전환 중에도 다음 애니메이션의 반복 설정을 별도로 사용
	const bool nextFinished =
		advance(desc.next, _nextLoop);

	desc.tweenSumTime += dt;

	if (desc.tweenDuration <= 0.f)
	{
		desc.tweenRatio = 1.f;
	}
	else
	{
		desc.tweenRatio =
			desc.tweenSumTime / desc.tweenDuration;

		if (desc.tweenRatio > 1.f)
			desc.tweenRatio = 1.f;
	}

	// 전환 중에는 외부에서 완료로 처리하지 않음
	_isAnimationFinished = false;

	if (desc.tweenRatio >= 1.f)
	{
		desc.curr = desc.next;

		// 다음 애니메이션이 현재 애니메이션이 되는 시점에 반영
		_loop = _nextLoop;

		desc.ClearNextAnim();
		desc.tweenSumTime = 0.f;
		desc.tweenRatio = 0.f;

		_isAnimationFinished = nextFinished;
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
	//if (_model == nullptr)
	//	return;
	//
	//TweenDesc& desc = _tweenDesc;
	//
	//shared_ptr<ModelAnimation> animation =
	//	_model->GetAnimationByIndex(desc.curr.animIndex);
	//
	//if (animation == nullptr)
	//	return;
	//
	//uint32 boneCount = _model->GetBoneCount();
	//
	//_currentBoneTransforms.resize(
	//	boneCount,
	//	Matrix::Identity);
	//
	//uint32 frameIndex = desc.curr.currFrame;
	//
	//if (frameIndex >= animation->frameCount)
	//	frameIndex = animation->frameCount - 1;
	//
	//for (uint32 b = 0; b < boneCount; b++)
	//{
	//	shared_ptr<ModelBone> bone =
	//		_model->GetBoneByIndex(b);
	//
	//	if (bone == nullptr)
	//		continue;
	//
	//	Matrix matAnimation =
	//		Matrix::Identity;
	//
	//	shared_ptr<ModelKeyframe> frame =
	//		animation->GetKeyframe(bone->name);
	//
	//	if (frame != nullptr)
	//	{
	//		if (frameIndex >= frame->transforms.size())
	//			continue;
	//
	//		ModelKeyframeData& data =
	//			frame->transforms[frameIndex];
	//
	//		Matrix S;
	//		Matrix R;
	//		Matrix T;
	//
	//		S = Matrix::CreateScale(
	//			data.scale.x,
	//			data.scale.y,
	//			data.scale.z);
	//
	//		R = Matrix::CreateFromQuaternion(
	//			data.rotation);
	//
	//		T = Matrix::CreateTranslation(
	//			data.translation.x,
	//			data.translation.y,
	//			data.translation.z);
	//
	//		matAnimation = S * R * T;
	//	}
	//
	//	Matrix parentMatrix =
	//		Matrix::Identity;
	//
	//	if (bone->parentIndex >= 0 &&
	//		bone->parentIndex < boneCount)
	//	{
	//		parentMatrix =
	//			_currentBoneTransforms[
	//				bone->parentIndex];
	//	}
	//
	//	_currentBoneTransforms[b] =
	//		matAnimation * parentMatrix;
	//}

	if (!_model)
		return;

	const uint32 boneCount = _model->GetBoneCount();

	if (boneCount == 0)
		return;

	// 특정 애니메이션의 특정 프레임에서
	// 부모 변환까지 포함한 본 행렬 계산
	auto buildPose = [&](
		int32 animIndex,
		uint32 frameIndex,
		vector<Matrix>& pose) -> bool
		{
			pose.assign(boneCount, Matrix::Identity);

			if (animIndex < 0)
				return false;

			auto animation =
				_model->GetAnimationByIndex(animIndex);

			if (!animation || animation->frameCount == 0)
				return false;

			if (frameIndex >= animation->frameCount)
				frameIndex = animation->frameCount - 1;

			for (uint32 i = 0; i < boneCount; ++i)
			{
				auto bone = _model->GetBoneByIndex(i);

				if (!bone)
					continue;

				Matrix local = Matrix::Identity;

				auto keyframe =
					animation->GetKeyframe(bone->name);

				if (keyframe &&
					frameIndex < keyframe->transforms.size())
				{
					const auto& data =
						keyframe->transforms[frameIndex];

					Matrix scale = Matrix::CreateScale(
						data.scale.x,
						data.scale.y,
						data.scale.z
					);

					Matrix rotation =
						Matrix::CreateFromQuaternion(data.rotation);

					Matrix translation = Matrix::CreateTranslation(
						data.translation.x,
						data.translation.y,
						data.translation.z
					);

					local = scale * rotation * translation;
				}

				// 
				if (bone->parentIndex >= 0 &&
					static_cast<uint32>(bone->parentIndex) < i)
				{
					pose[i] =
						local * pose[bone->parentIndex];
				}
				else
				{
					pose[i] = local;
				}
			}

			return true;
		};

	auto clampRatio = [](float value)
		{
			if (value < 0.f) return 0.f;
			if (value > 1.f) return 1.f;
			return value;
		};

	//
	auto samplePose = [&](
		const auto& frame,
		vector<Matrix>& result) -> bool
		{
			vector<Matrix> poseA;
			vector<Matrix> poseB;

			if (!buildPose(
				frame.animIndex, frame.currFrame, poseA))
			{
				return false;
			}

			if (!buildPose(
				frame.animIndex, frame.nextFrame, poseB))
			{
				return false;
			}

			const float ratio = clampRatio(frame.ratio);

			result.resize(boneCount);

			for (uint32 i = 0; i < boneCount; ++i)
			{
				result[i] =
					Matrix::Lerp(poseA[i], poseB[i], ratio);
			}

			return true;
		};

	vector<Matrix> currentPose;

	if (!samplePose(_tweenDesc.curr, currentPose))
		return;

	if (_tweenDesc.next.animIndex >= 0)
	{
		vector<Matrix> nextPose;

		if (samplePose(_tweenDesc.next, nextPose))
		{
			const float ratio =
				clampRatio(_tweenDesc.tweenRatio);

			for (uint32 i = 0; i < boneCount; ++i)
			{
				currentPose[i] = Matrix::Lerp(
					currentPose[i],
					nextPose[i],
					ratio
				);
			}
		}
	}

	_currentBoneTransforms = std::move(currentPose);
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

	const AnimData& animData = iter->second;
	TweenDesc& desc = _tweenDesc;

	// 이미 해당 애니메이션으로 전환 중이라면 유지
	// 전환 시간을 다시 초기화하지 않음
	if (desc.next.animIndex == animData.animIndex)
		return;

	// 현재 애니메이션을 요청했다면 다른 전환 예약을 취소
	// 예: Idle → Move 전환 중 다시 Idle 요청
	if (desc.curr.animIndex == animData.animIndex)
	{
		if (desc.next.animIndex >= 0)
		{
			desc.ClearNextAnim();

			desc.tweenSumTime = 0.f;
			desc.tweenRatio = 0.f;
		}

		_loop = animData.animLoop;
		desc.curr.speed = animData.speed;
		return;
	}

	// 새로운 애니메이션으로 전환
	auto animation =
		_model->GetAnimationByIndex(animData.animIndex);

	if (animation == nullptr || animation->frameCount == 0)
		return;

	desc.next.animIndex = animData.animIndex;
	desc.next.currFrame = 0;
	desc.next.nextFrame = animation->frameCount > 1 ? 1 : 0;
	desc.next.sumTime = 0.f;
	desc.next.ratio = 0.f;
	desc.next.speed = animData.speed;

	desc.tweenSumTime = 0.f;
	desc.tweenRatio = 0.f;
	desc.tweenDuration = 0.2f;

	//_loop = animData.animLoop;
	//_isAnimationFinished = false;
	_nextLoop = animData.animLoop;
	_isAnimationFinished = false;
}

bool ModelAnimator::IsAnimationFinished()
{
	//return _isAnimationFinished;;

	// 다른 애니메이션으로 전환 중이면 아직 끝난 것이 아님
	if (_tweenDesc.next.animIndex >= 0)
		return false;

	return _isAnimationFinished;
}

bool ModelAnimator::GetAnimationProgress(const string& animName, float& progress)
{
	progress = 0.f;

	if (_model == nullptr)
		return false;

	auto iter = _animDataMap.find(animName);

	if (iter == _animDataMap.end())
		return false;

	const int32 animIndex = iter->second.animIndex;

	// 공격으로 전환 중이면 next의 시간을 사용
	const auto* frame = &_tweenDesc.curr;

	if (_tweenDesc.next.animIndex == animIndex)
	{
		frame = &_tweenDesc.next;
	}
	else if (_tweenDesc.curr.animIndex != animIndex)
	{
		return false;
	}

	auto animation = _model->GetAnimationByIndex(animIndex);

	if (animation == nullptr || animation->frameCount == 0)
		return false;

	if (animation->frameCount == 1)
	{
		progress = 1.f;
		return true;
	}

	float framePosition =
		static_cast<float>(frame->currFrame);

	if (frame->nextFrame == frame->currFrame + 1)
	{
		framePosition += frame->ratio;
	}

	progress = framePosition /
		static_cast<float>(animation->frameCount - 1);

	if (progress < 0.f) progress = 0.f;
	if (progress > 1.f) progress = 1.f;

	return true;
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

			//Matrix matAnimation;
			//
			//shared_ptr<ModelKeyframe> frame = animation->GetKeyframe(bone->name);

			Matrix bindLocal = bone->transform;

			if (bone->parentIndex >= 0)
			{
				auto parent =
					_model->GetBoneByIndex(bone->parentIndex);

				if (parent)
				{
					bindLocal =
						bone->transform * parent->transform.Invert();
				}
			}

			Matrix matAnimation = bindLocal;

			auto frame = animation->GetKeyframe(bone->name);




			if (frame != nullptr && f < frame->transforms.size())
			{
				ModelKeyframeData& data = frame->transforms[f];

				Matrix S, R, T;
				S = Matrix::CreateScale(data.scale.x, data.scale.y, data.scale.z);
				R = Matrix::CreateFromQuaternion(data.rotation);
				T = Matrix::CreateTranslation(data.translation.x, data.translation.y, data.translation.z);

				matAnimation = S * R * T;
			}
			//else
			//{
			//	matAnimation = Matrix::Identity;
			//}

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
