#pragma once
#include "Character.h"
#include "Enemy.h"
#include "GameObject.h"
#include "ModelAnimation.h"

class CLD2 : public Enemy
{
public:
    CLD2();
    virtual ~CLD2();

public:
    virtual void Init() override;
    virtual void Update() override;

};

