#include "DX12LibPCH.h"

#include "Application.h"
#include "DX12Lib/Game.h"
#include "DX12Lib/Window.h"
#include "WindowPersist.h"

WindowPersist g_windowPersist;

Game::Game()
{
}

Game::~Game()
{
    assert(!m_pWindow && "Use Game::Destroy() before destruction.");
}

int Game::GetClientWidth() const
{
	assert(m_pWindow);
	return std::max(1, (*m_pWindow).GetClientWidth());
//	assert(m_Width == (*m_pWindow).GetClientWidth());
//	return std::max(1, m_Width);
}

int Game::GetClientHeight() const
{
	assert(m_pWindow);
	return std::max(1, (*m_pWindow).GetClientHeight());
//	assert(m_Height == (*m_pWindow).GetClientHeight());
//	return std::max(1, m_Height);
}

bool Game::Initialize()
{
    // Check for DirectX Math library support.
//    if (!DirectX::XMVerifyCPUSupport())
//    {
//        MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
//        return false;
//    }

	if (!g_windowPersist.IsValid())
	{
		g_windowPersist = {};
		g_windowPersist.data[0] = g_windowPersist.data[1] = 100;
		g_windowPersist.data[2] = 1280;
		g_windowPersist.data[3] = 768;
	}
	
	m_pWindow = Application::Get().CreateRenderWindow(L"KosmoRay2", 1024, 768, false);

	g_windowPersist.ApplyState(m_pWindow->GetWindowHandle());

    m_pWindow->RegisterCallbacks(shared_from_this());
    m_pWindow->Show();

    return true;
}

void Game::Destroy()
{
    Application::Get().DestroyWindow(m_pWindow);
    m_pWindow.reset();
}

void Game::OnUpdate(UpdateEventArgs& e)
{

}

void Game::OnRender(RenderEventArgs& e)
{

}

void Game::OnKeyPressed(KeyEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnKeyReleased(KeyEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseMoved(class MouseMotionEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonPressed(MouseButtonEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseButtonReleased(MouseButtonEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnMouseWheel(MouseWheelEventArgs& e)
{
    // By default, do nothing.
}

void Game::OnResize(ResizeEventArgs& e)
{
}

void Game::OnWindowDestroy()
{
    // If the Window which we are registered to is 
    // destroyed, then any resources which are associated 
    // to the window must be released.
    UnloadContent();
}

