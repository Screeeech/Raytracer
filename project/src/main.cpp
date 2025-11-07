// External includes
#include "SDL.h"
#include "SDL_surface.h"
#undef main

// Standard includes
#include <iostream>

// Project includes
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Timer.hpp"
#if defined(_DEBUG)
#include "LeakDetector.hpp"
#endif

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}

int main(int argc, char* args[])
{
    // Unreferenced parameters
    (void)argc;
    (void)args;

// Leak detection
#if defined(_DEBUG)
    LeakDetector detector{};
#endif

    // Create window + surfaces
    SDL_Init(SDL_INIT_VIDEO);

    const uint32_t width = 640;
    const uint32_t height = 480;

    SDL_Window* pWindow =
        SDL_CreateWindow("RayTracer - Lily Botha", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);

    if(!pWindow)
        return 1;

    // Initialize "framework"
    auto* const pTimer = new Timer();
    auto* const pRenderer = new Renderer(pWindow);

    // auto* const pScene = new Scene_W1();
    auto* const pScene = new Scene_W4_ReferenceScene();
    pScene->Initialize();

    // Start loop
    pTimer->Start();

    // Start Benchmark
    // pTimer->StartBenchmark();

    float printTimer = 0.F;
    bool isLooping = true;
    bool takeScreenshot = false;
    while(isLooping)
    {
        //--------- Get input events ---------
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
                case SDL_QUIT:
                    isLooping = false;
                    break;
                case SDL_KEYUP:
                    if(e.key.keysym.scancode == SDL_SCANCODE_X)
                        takeScreenshot = true;
                    break;
                default:
                    break;
            }
            pRenderer->ProcessInput(e);
        }

        //--------- Update ---------
        pScene->Update(pTimer);

        //--------- Render ---------
        pRenderer->Render(pScene);

        //--------- Timer ---------
        pTimer->Update();
        printTimer += pTimer->GetElapsed();
        if(printTimer >= 1.F)
        {
            printTimer = 0.F;
            std::cout << "dFPS: " << pTimer->GetdFPS() << '\n';
        }

        // Save screenshot after full render
        if(takeScreenshot)
        {
            if(!pRenderer->SaveBufferToImage())
                std::cout << "Screenshot saved!" << '\n';
            else
                std::cout << "Something went wrong. Screenshot not saved!" << '\n';
            takeScreenshot = false;
        }
    }
    pTimer->Stop();

    // Shutdown "framework"
    delete pScene;
    delete pRenderer;
    delete pTimer;

    ShutDown(pWindow);
    return 0;
}
