#pragma once
#include "Character.h"
#include "Enemy.h"
#include "GameObject.h"
#include "ModelAnimation.h"

class CLD3 : public Enemy
{
public:
    CLD3();
    virtual ~CLD3();

public:
    virtual void Init() override;
    virtual void Update() override;

};

