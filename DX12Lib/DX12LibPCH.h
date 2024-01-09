#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

#include <shellapi.h> // For CommandLineToArgvW

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined(CreateWindow)
#undef CreateWindow
#endif

// Windows Runtime Library. Needed for ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// see https://github.com/microsoft/DirectX-Graphics-Samples/issues/567
#include <initguid.h>
// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>
#include <memory>

// Helper functions
#include "DX12Lib/Helpers.h"
