#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>

#include "DX12Lib/Application.h"
#include "App2.h"
#include "App3.h"

#include <dxgidebug.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "Shlwapi.lib") // PathRemoveFileSpecW()
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

void ReportLiveObjects()
{
//later    IDXGIDebug1* dxgiDebug;
//later    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

//later    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
//later    dxgiDebug->Release();
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    int retCode = 0;

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE hModule = GetModuleHandleW(NULL);
    if ( GetModuleFileNameW(hModule, path, MAX_PATH) > 0 )
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

    Application::Create(hInstance);
    {
//        std::shared_ptr<App2> demo = std::make_shared<App2>(L"KosmoRay2 App2 DX12", 1280, 720);
        std::shared_ptr<App3> demo = std::make_shared<App3>(L"KosmoRay2 App3 DX12", 1280, 720);
        retCode = Application::Get().Run(demo);
    }
    Application::Destroy();

    atexit(&ReportLiveObjects);

    return retCode;
}