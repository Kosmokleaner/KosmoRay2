#pragma once

#include <assert.h>

#include "Mathlib.h"

// for now we want all code to be able reference DX12
// If we abstract this ll away this can go into a smaller scope
#include <d3d12.h>
#undef min
#undef max
//#include <d3dx12.h>
#include <dxgi1_5.h> // IDXGISwapChain4
#include <dxgi1_6.h> // IDXGIAdapter4
#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;


// less typing
class Renderer;
