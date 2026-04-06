#include "pch.h"
#include "GameManager.h"
#include "SceneManager.h"

#include "TownScene.h"

// =========================
// ½Ì±ÛÅæ
// =========================
GameManager& GameManager::Get()
{
    static GameManager instance;
    return instance;
}

void GameManager::Init()
{
    Scene = make_shared<TownScene>();
    Scene->Start();
}

// =========================
// ¾÷µ¥ÀÌÆ®
// =========================
void GameManager::Update()
{
}