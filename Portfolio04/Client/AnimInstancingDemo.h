#pragma once
#include "IExecute.h"

class AnimInstancingDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	shared_ptr<Shader> _shader;
	shared_ptr<GameObject> _camera;
	vector<shared_ptr<GameObject>> _objs;

private:
};

