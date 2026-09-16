#pragma once
#include "GameObject.h"

class Model;
class Shader;

class Map : public GameObject
{
public:
    void Init(shared_ptr<Shader> shader, shared_ptr<Shader> debugShader);

private:
    void CreateWallColliders(shared_ptr<Model> model, shared_ptr<Shader> debugShader, const Matrix& mapWorld);
    void CreateSlopeColliders(shared_ptr<Model> model, shared_ptr<Shader> debugShader, const Matrix& mapWorld);

};

