#pragma once
#include "GameObject.h"

class Character : public GameObject
{
public:
    Character();
    virtual ~Character();

public:
    virtual void Init();
    virtual void Update() override;

    void InitCharacter();
        
public:
    shared_ptr<class CharacterMovement> GetCharacterMovement()
    {
        return _movement;
    }

    shared_ptr<class HealthComponent> GetHealthComponent()
    {
        return _health;
    }

protected:
    shared_ptr<Shader> _shader;
    shared_ptr<Shader> _debugShader;
    shared_ptr<CharacterMovement> _movement;
    shared_ptr<HealthComponent> _health;
};

