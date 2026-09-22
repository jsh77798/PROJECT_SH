#include "pch.h"
#include "Main.h"
#include "Engine/Game.h"
#include "GameManager.h"
#include "SceneManager.h"

#include "SnowDemo.h"
#include "AnimInstancingDemo.h"
#include "RawBufferDemo.h"
#include "TextureBufferDemo.h"
#include "StructuredBufferDemo.h"
#include "ViewportDemo.h"
#include "GroupDemo.h"
#include "TownScene.h"

#include "AppManager.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	GameDesc desc;
	desc.appName = L"Portfolio04";
	desc.hInstance = hInstance;
	desc.vsync = false;
	desc.hWnd = NULL;
	//desc.width = 1920;
	//desc.height = 1080;
	desc.width = GetSystemMetrics(SM_CXSCREEN);
	desc.height = GetSystemMetrics(SM_CYSCREEN);
	desc.clearColor = Color(0.f, 0.f, 0.f, 0.f);
	desc.app = make_shared<AppManager>();
	
	GAME->Run(desc);

	
	return 0;
}