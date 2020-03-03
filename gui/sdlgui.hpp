#pragma once
#include <SDL2/SDL.h>
#include <mutex>
#include <condition_variable>
#include "nes.hpp"

typedef int64_t i64;

using namespace std;

class GuiSDL {
public:
    // wndPtr is used for initializaing SDL from an existing window
    GuiSDL(u16 w, u16 h, void* wndPtr = nullptr);
    ~GuiSDL();
    void render(Frame* frame);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u16 width;
    u16 height;
};
