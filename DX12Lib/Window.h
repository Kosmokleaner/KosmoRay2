/**
* @brief A window for our application.
*/
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

#include <d3d12.h>
#undef min
#undef max

#include <dxgi1_5.h>

#include <wrl.h> // ComPtr<>
using namespace Microsoft::WRL;


#include "DX12Lib/Events.h"
#include "DX12Lib/HighResolutionClock.h"
#include "DX12Lib/DescriptorHeap.h"

// Forward-declare the DirectXTemplate class.
class Game;

class Window
{
public:
    // Number of swapchain back buffers.
    static const UINT BufferCount = 3;

    /**
    * Get a handle to this window's instance.
    * @returns The handle to the window instance or nullptr if this is not a valid window.
    */
    HWND GetWindowHandle() const;

    /**
    * Destroy this window.
    */
    void Destroy();

    const std::wstring& GetWindowName() const;

    int GetClientWidth() const;
    int GetClientHeight() const;

    /**
    * Should this window be rendered with vertical refresh synchronization.
    */
    bool IsVSync() const;
    bool isActive() const;
    void SetVSync(bool vSync);
    void ToggleVSync();

    /**
    * Is this a windowed window or full-screen?
    */
    bool IsFullScreen() const;

    // Set the fullscreen state of the window.
    void SetFullscreen(bool fullscreen);
    void ToggleFullscreen();

    void Show();

    void Hide();

    UINT GetCurrentBackBufferIndex() const;

    /**
     * Present the swapchain's back buffer to the screen.
     * Returns the current back buffer index after the present.
     */
    UINT Present();

    /**
     * Get the render target view for the current back buffer.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;

    ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

    DXGI_FORMAT GetBackBufferFormat() const;

protected:
    // The Window procedure needs to call protected methods of this class.
    friend LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Only the application can create a window.
    friend class Application;
    // The DirectXTemplate class needs to register itself with a window.
    friend class Game;

    Window() = delete;
    Window(HWND hWnd, const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync );
    virtual ~Window();

    // Register a Game with this window. This allows
    // the window to callback functions in the Game class.
    void RegisterCallbacks( std::shared_ptr<Game> pGame );

    // Update and Draw can only be called by the application.
    virtual void OnUpdate(UpdateEventArgs& e);
    virtual void OnRender(RenderEventArgs& e);

    // A keyboard key was pressed
    virtual void OnKeyPressed(KeyEventArgs& e);
    // A keyboard key was released
    virtual void OnKeyReleased(KeyEventArgs& e);

    // The mouse was moved
    virtual void OnMouseMoved(MouseMotionEventArgs& e);
    // A button on the mouse was pressed
    virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);
    // A button on the mouse was released
    virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);
    // The mouse wheel was moved.
    virtual void OnMouseWheel(MouseWheelEventArgs& e);

    // The window was resized.
    virtual void OnResize(ResizeEventArgs& e);

    // Create the swapchian.
    ComPtr<IDXGISwapChain4> CreateSwapChain();

    // Update the render target views for the swapchain back buffers.
    void UpdateRenderTargetViews();

private:
    // Windows should not be copied.
    Window(const Window& copy) = delete;
    Window& operator=(const Window& other) = delete;

    HWND m_hWnd = 0;

    std::wstring m_WindowName;
    
    int m_ClientWidth = 0;
    int m_ClientHeight = 0;
    bool m_VSync = false;
    bool m_Fullscreen = false;

    HighResolutionClock m_UpdateClock;
    HighResolutionClock m_RenderClock;
    uint64_t m_FrameCounter = 0;

    std::weak_ptr<Game> m_pGame;

    ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
    DescriptorHeap RTVDescriptorHeap;
    ComPtr<ID3D12Resource> m_d3d12BackBuffers[BufferCount];

    UINT m_RTVDescriptorSize = 0;
    UINT m_CurrentBackBufferIndex = 0;

    RECT m_WindowRect;
    bool m_IsTearingSupported = false;

};