#include "gui/sdlgui.hpp"
#include <iostream>

GuiSDL::GuiSDL(u16 w, u16 h, PPU* _ppu, NES& _nes, void* wndPtr)
    : width{w}, height{h}, ppu{_ppu}, nes{_nes}
{
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    if(wndPtr == nullptr) window = SDL_CreateWindow("HaniwaNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    else window = SDL_CreateWindowFrom(reinterpret_cast<void*>(wndPtr));
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING, width, height);
}

GuiSDL::~GuiSDL() {
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void GuiSDL::render() {
    SDL_UpdateTexture(texture, NULL, &(ppu->publicImage()[0]), width * 4);
    if(ppu->scanline >= 241) {
        std::cout << "HERE\n";
    }
    SDL_RenderCopy( renderer, texture, NULL, NULL );
    SDL_RenderPresent( renderer );

}

