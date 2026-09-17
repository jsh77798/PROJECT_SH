#pragma once
#include "Character.h"
#include "Enemy.h"
#include "GameObject.h"
#include "ModelAnimation.h"

class Dog : public Enemy
{
public:
    Dog();
    virtual ~Dog();

public:
    virtual void Init() override;
    virtual void Update() override;

};

