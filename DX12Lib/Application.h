/**
* The application class is used to create windows for our application.
*/
#pragma once

#include "Renderer.h"

#include <memory>
#include <string>

class Window;
class Game;
class CommandQueue;

class Application
{
public:

    static void Create(HINSTANCE hInst);
    static void Destroy();

    static Application& Get();

    /**
    * @returns The created window instance. If an error occurred while creating the window an invalid
    * window instance is returned. If a window with the given name already exists, that window will be
    * returned.
    */
    std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true );

    void DestroyWindow(std::shared_ptr<Window> window);

    std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

    /**
    * Run the application loop and message pump.
    * @return The error code if an error occurred.
    */
    int Run(std::shared_ptr<Game> pGame);

    /**
    * Request to quit the application and close all windows.
    * @param exitCode The error code to return to the invoking process.
    */
    void Quit(int exitCode = 0);

    Renderer renderer;

protected:

    // Create an application instance.
    Application(HINSTANCE hInst);
    // Destroy the application instance and all windows associated with this application.
    virtual ~Application();

private:
    Application(const Application& copy) = delete;
    Application& operator=(const Application& other) = delete;

    // The application instance handle that this application was created with.
    HINSTANCE m_hInstance;
};