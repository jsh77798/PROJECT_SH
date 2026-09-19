#pragma once
#include "GameObject.h"

class Model;
class Shader;

class Map : public GameObject
{
public:
    void Init(shared_ptr<Shader> shader, shared_ptr<Shader> debugShader);

    shared_ptr<Model> GetModel() const
    {
        return _model;
    }

private:
    void CreateWallColliders(shared_ptr<Model> model, shared_ptr<Shader> debugShader, const Matrix& mapWorld);
    void CreateSlopeColliders(shared_ptr<Model> model, shared_ptr<Shader> debugShader, const Matrix& mapWorld);

private:
    shared_ptr<Model> _model;
};

