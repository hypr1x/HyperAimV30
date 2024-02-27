#include "gui.h"
#include <iostream>
#include <fstream>
#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_dx9.h"
#include "./imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, 
	UINT msg, 
	WPARAM wParam, 
	LPARAM lParam
);

static long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;
	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU)
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter);
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);

		}
	}return 0;

	}

	return DefWindowProcW(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName, const char* className) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = className;
	windowClass.hIconSm = 0;

	RegisterClassExA(&windowClass);

	window = CreateWindowA(
		className,
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestoryHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;
	
	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	
	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		d3d = nullptr;
	}

	if (d3d)
	{
		device->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsClassic();
	
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	
	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}
int fov = 410;
int strength = 300;
bool circle = false;
bool theme = false;
int threshold = 65;
int first = 1;

void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"Hyper Aim Official",
		&exit,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);
	ImGui::TextColored({ 0, 255, 0, 255 }, "Status Undetected");
	ImGui::TextColored({ 0, 255, 20, 255 }, "Discord yohypr");
	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Aim Assist"))
	{
		ImGui::Spacing();
		ImGui::Text("Aim Assist Fov");
		ImGui::Spacing();
		ImGui::SliderInt("Fov (Pixel radius)", &fov, 1, 1000, "%d", 0);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Aim Assist Strength");
		ImGui::Spacing();
		ImGui::SliderInt("Strength (Multiplier)", &strength, 1, 2000, "%d", 0);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Aim Threshold");
		ImGui::Spacing();
		ImGui::SliderInt("Confidence (Threshold)", &threshold, 1, 100, "%d", 0);
	}
	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Visuals"))
	{
		ImGui::Spacing();
		if (ImGui::BeginTable("split", 3))
		{
			ImGui::TableNextColumn();
			ImGui::Checkbox("Fov Circle", &circle);
			ImGui::TableNextColumn();
			ImGui::Checkbox("Open Debug", &gui::open);
			ImGui::TableNextColumn();
			ImGui::Checkbox("Dark Theme", &theme);
			if (theme)
				ImGui::StyleColorsDark();
			else
				ImGui::StyleColorsClassic();

			ImGui::EndTable();
		}
	}
	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Keybinds"))
	{
		ImGui::Spacing();
		if (ImGui::BeginTable("", 3))
		{
			ImGui::TableNextColumn();
			ImGui::Button("Toggle Key", { int(500 / 3)-10, 30 });
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Left Alt");
			}
			ImGui::Spacing();
			ImGui::TableNextColumn();
			ImGui::Button("Aim Key", { int(500 / 3)-10, 30 });
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Right CLick");
			}
			ImGui::Spacing();
			ImGui::TableNextColumn();
			ImGui::Button("Panic Key", { int(500 / 3)-10, 30 });
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("F2 Key");
			}
			ImGui::Spacing();
			ImGui::TableNextColumn();
			ImGui::Button("Freeze Time", { int(500 / 3) - 10, 30 });
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Enter (option must be enabled)");
			}
			ImGui::Spacing();
			ImGui::EndTable();

		}
	}
	ImGui::Spacing();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Miscellaneous"))
	{
		ImGui::Spacing();
		ImGui::Checkbox("Freeze Time (Fully Undetectable)", &gui::freeze);
	}
	
	if (first == 1)
	{
		ImGui::GetStateStorage()->SetInt(ImGui::GetID("Aim Assist"), first);
		ImGui::GetStateStorage()->SetInt(ImGui::GetID("Visuals"), first);
		ImGui::GetStateStorage()->SetInt(ImGui::GetID("Keybinds"), first);
		ImGui::GetStateStorage()->SetInt(ImGui::GetID("Miscellaneous"), first);
		first = 0;
	}
	std::ofstream outFile("config");
	if (outFile.is_open())
		outFile << fov << " " << strength << " " << int(circle) << " " << threshold;
	//ImGui::ShowDemoWindow();
	ImGui::End();

}
