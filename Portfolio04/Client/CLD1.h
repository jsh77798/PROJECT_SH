#pragma once
#include "Character.h"
#include "Enemy.h"
#include "GameObject.h"
#include "ModelAnimation.h"

class CLD1 : public Enemy
{
public:
    CLD1();
    virtual ~CLD1();

public:
    virtual void Init() override;
    virtual void Update() override;

};

