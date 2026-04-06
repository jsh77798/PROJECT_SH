#pragma once
#include "GameObject.h"

class Player : public GameObject
{
public:
    Player();
    virtual ~Player();

public:
    void Init();
    void Update();

private:
    shared_ptr<GameObject> _obj;
    shared_ptr<GameObject> _camera;
};

