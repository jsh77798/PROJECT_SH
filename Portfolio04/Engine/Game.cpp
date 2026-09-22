#include "pch.h"
#include "Game.h"
#include "IExecute.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WPARAM Game::Run(GameDesc& desc)
{
	_desc = desc;
	assert(_desc.app != nullptr);

	// 1) 윈도우 창 정보 등록
	MyRegisterClass();

	// 2) 윈도우 창 생성
	if (!InitInstance(SW_SHOWNORMAL))
		return FALSE;

	GRAPHICS->Init(_desc.hWnd);
	TIME->Init();
	INPUT->Init(_desc.hWnd);
	GUI->Init();
	// 커서는 WndProc에서 관리
	ImGui::GetIO().ConfigFlags |=
		ImGuiConfigFlags_NoMouseCursorChange;

	// ImGui가 직접 그리는 커서도 비활성화
	ImGui::GetIO().MouseDrawCursor = false;
	RESOURCES->Init();

	_desc.app->Init();

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			Update();
		}
	}

	return msg.wParam;
}


ATOM Game::MyRegisterClass()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = _desc.hInstance;
	wcex.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _desc.appName.c_str();
	wcex.hIconSm = wcex.hIcon;

	return RegisterClassExW(&wcex);
}

BOOL Game::InitInstance(int cmdShow)
{
	//RECT windowRect = { 0, 0, _desc.width, _desc.height };
	//::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);
	//
	//_desc.hWnd = CreateWindowW(_desc.appName.c_str(), _desc.appName.c_str(), WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, _desc.hInstance, nullptr);
	//
	//if (!_desc.hWnd)
	//	return FALSE;
	//
	//::ShowWindow(_desc.hWnd, cmdShow);
	//::UpdateWindow(_desc.hWnd);
	//
	//return TRUE;

	POINT point = { 0, 0 };

	HMONITOR monitor = ::MonitorFromPoint(
		point,
		MONITOR_DEFAULTTOPRIMARY
	);

	MONITORINFO monitorInfo = {};
	monitorInfo.cbSize = sizeof(monitorInfo);

	if (!::GetMonitorInfo(monitor, &monitorInfo))
		return FALSE;

	const RECT& rect = monitorInfo.rcMonitor;

	_desc.width = rect.right - rect.left;
	_desc.height = rect.bottom - rect.top;

	// 테두리가 없으므로 AdjustWindowRect는 사용하지 않음
	_desc.hWnd = ::CreateWindowW(
		_desc.appName.c_str(),
		_desc.appName.c_str(),
		WS_POPUP,
		rect.left,
		rect.top,
		_desc.width,
		_desc.height,
		nullptr,
		nullptr,
		_desc.hInstance,
		nullptr
	);

	if (!_desc.hWnd)
		return FALSE;

	::ShowWindow(_desc.hWnd, cmdShow);
	::UpdateWindow(_desc.hWnd);

	return TRUE;
}

LRESULT CALLBACK Game::WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (ImGui_ImplWin32_WndProcHandler(handle, message, wParam, lParam))
	//	return true;
	//
	//switch (message)
	//{
	//case WM_SIZE:
	//	break;
	//case WM_CLOSE:
	//case WM_DESTROY:
	//	PostQuitMessage(0);
	//	break;
	//default:
	//	return ::DefWindowProc(handle, message, wParam, lParam);
	//}

	// ESC로 종료 요청
	if (message == WM_KEYDOWN && wParam == VK_ESCAPE)
	{
		::ShowWindow(handle, SW_MINIMIZE);
		return 0;
	}

	// 게임 화면 안에서는 마우스 커서 숨김
	if (message == WM_SETCURSOR &&
		LOWORD(lParam) == HTCLIENT)
	{
		::SetCursor(nullptr);
		return TRUE;
	}

	// 종료 메시지는 ImGui보다 먼저 처리
	switch (message)
	{
	case WM_CLOSE:
		::DestroyWindow(handle);
		return 0;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	if (ImGui_ImplWin32_WndProcHandler(
		handle,
		message,
		wParam,
		lParam))
	{
		return 1;
	}

	switch (message)
	{
	case WM_SIZE:
		return 0;

	default:
		return ::DefWindowProc(
			handle,
			message,
			wParam,
			lParam
		);
	}
}

void Game::Update()
{
	TIME->Update();
	INPUT->Update();
	ShowFps();

	GRAPHICS->RenderBegin();

	SCENE->Update();

	GUI->Update();
	//_desc.app->Update();
	//_desc.app->Render();
	GUI->Render();

	GRAPHICS->RenderEnd();
}

void Game::ShowFps()
{
	uint32 fps = GET_SINGLE(TimeManager)->GetFps();

	WCHAR text[100] = L"";
	::wsprintf(text, L"FPS : %d", fps);

	::SetWindowText(_desc.hWnd, text);

}

