#include "pch.h"
#include "Character.h"
#include "CharacterMovement.h"

Character::Character()
{
}

Character::~Character()
{
}

void Character::Init()
{
    
}

void Character::Update()
{
    GameObject::Update();
}

void Character::InitCharacter()
{
    _shader = make_shared<Shader>(L"SkinnedLit.fx");
    _debugShader = make_shared<Shader>(L"Debug.fx");

    _movement = make_shared<CharacterMovement>();
    AddComponent(_movement);
}
