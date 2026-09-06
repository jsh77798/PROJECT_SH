#pragma once
#include "Component.h"

class Model;

struct AnimTransform
{
	// [ ][ ][ ][ ][ ][ ][ ] ... 250°³
	using TransformArrayType = array<Matrix, MAX_MODEL_TRANSFORMS>;
	// [ ][ ][ ][ ][ ][ ][ ] ... 500 °³
	array<TransformArrayType, MAX_MODEL_KEYFRAMES> transforms;
};

struct AnimData
{
	string name = "none";
	int32 animIndex = -1;
	bool animLoop = true;
	float speed = 1.f;
	bool canMove = true;
};

class ModelAnimator : public Component
{
	using Super = Component;

public:
	ModelAnimator(shared_ptr<Shader> shader);
	~ModelAnimator();

	void SetModel(shared_ptr<Model> model);
	void SetPass(uint8 pass) { _pass = pass; }
	shared_ptr<Shader> GetShader() { return _shader; }

	string MakeAnimData(string name, int32 animIndex, bool animLoop = true, float speed = 1.f, bool canMove = true)
	{
		AnimData animeData;
		animeData.name = name;
		animeData.animIndex = animIndex;
		animeData.animLoop = animLoop;
		animeData.speed = speed;
		animeData.canMove = canMove;
		_animDataMap[name] = animeData;

		return animeData.name;
	}

	virtual void Update() override;

	void UpdateTweenData();
	void RenderInstancing(shared_ptr<class InstancingBuffer>& buffer);
	InstanceID GetInstanceID();
	TweenDesc& GetTweenDesc() { return _tweenDesc; }

	void Play(string animName);
	bool IsAnimationFinished();

private:
	void CreateTexture();
	void CreateAnimationTransform(uint32 index);

private:
	vector<AnimTransform> _animTransforms;
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _srv;
	unordered_map<string, AnimData> _animDataMap;
	bool _loop = true;
	bool _isAnimationFinished = false;

private:
	TweenDesc _tweenDesc;

private:
	shared_ptr<Shader>	_shader;
	uint8				_pass = 0;
	shared_ptr<Model>	_model;
};

