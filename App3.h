#pragma once

#include "DX12Lib/Game.h"

class App3 : public Game
{

    void CreateRootSignatures();

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;
};

