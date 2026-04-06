#include "pch.h"
#include "AppManager.h"
#include "GameManager.h"

AppManager::AppManager()
{
}

AppManager::~AppManager()
{
}

void AppManager::Init()
{
	game = make_shared<GameManager>();
	game->Init();
}

void AppManager::Update()
{
}

void AppManager::Render()
{
}
