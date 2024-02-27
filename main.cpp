#include <windows.h>
#include "gui.h"
#include <thread>

HWND hwnd;
HWND mhwnd;
DWORD processId;

static void pyStart()
{
    system("python HyperAim.py");
}

static void cleanupFunction() {
    PostMessage(hwnd, WM_CLOSE, 0, 0);
}

int __stdcall wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    std::thread py(pyStart);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    hwnd = GetForegroundWindow();
    if (hwnd != NULL) 
        if (IsWindow(hwnd)) 
            ShowWindow(hwnd, SW_HIDE);
    std::atexit(cleanupFunction);
    gui::CreateHWindow("Hyper Aim", "Hyper Aim Class");
    gui::CreateDevice();
    gui::CreateImGui();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    mhwnd = GetForegroundWindow();
    
    if (mhwnd)
        SetWindowPos(mhwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    while (true)
    {
        if (GetAsyncKeyState(0x0D) != 0 && gui::freeze)
        {
            keybd_event(0x4B, 0, 0, 0);
            keybd_event(0x4B, 0, KEYEVENTF_KEYUP, 0);
            continue;
        }
        if (GetAsyncKeyState(VK_DELETE) != 0)
            if (mhwnd)
                ShowWindow(mhwnd, SW_HIDE);
        if (GetAsyncKeyState(VK_INSERT) != 0)
            if (mhwnd) {
                ShowWindow(mhwnd, SW_SHOW);
                SetWindowPos(mhwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            } 

        
        gui::BeginRender();
        gui::Render();
        gui::EndRender();
        if (GetAsyncKeyState(VK_F2) != 0)
        {
            if (IsWindow(hwnd)) {
                if (hwnd)
                {
                    GetWindowThreadProcessId(hwnd, &processId);
                    if (processId != 0) {
                        HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
                        if (processHandle != NULL)
                        {
                            TerminateProcess(processHandle, 0);
                        }
                    }
                }   
            }
            else
                return EXIT_SUCCESS;
        }
        if (hwnd)
            if (gui::open)
                ShowWindow(hwnd, SW_SHOW);
            else
                ShowWindow(hwnd, SW_HIDE);

        if (gui::freeze != true)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestoryHWindow();

    return EXIT_SUCCESS;
}