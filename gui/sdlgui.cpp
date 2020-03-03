#include "gui/sdlgui.hpp"
#include <iostream>

/*
    SDL is used only for rendering.
    Other parts of GUI are implemented with Qt.
*/
GuiSDL::GuiSDL(u16 w, u16 h, void* wndPtr)
    : width{w}, height{h}
{
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    if(wndPtr == nullptr) window = SDL_CreateWindow("HaniwaNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    else window = SDL_CreateWindowFrom(reinterpret_cast<void*>(wndPtr));
    // SDL_RENDERER_PRESENTVSYNC, in my case, seems to be crucial to image's smoothness
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING, width, height);

    // filling renderer with black
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

GuiSDL::~GuiSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GuiSDL::render(Frame* frame) {
    if(frame) {
        SDL_UpdateTexture(texture, NULL, frame, width * 4);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
}

