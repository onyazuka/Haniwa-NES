#pragma once
#include <SDL2/SDL.h>
#include <mutex>
#include <condition_variable>
#include "nes.hpp"

using namespace std;

class GuiSDL : public Observer<PPU> {
public:
    GuiSDL(u16 w, u16 h, PPU* _ppu, NES& _nes);
    ~GuiSDL();
    void update(PPU*, int);
    void eventLoop();

private:
    void render();
    void fetchEvents();

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u16 width;
    u16 height;
    PPU* ppu;
    NES& nes;

    std::mutex redrawMtx;
    std::condition_variable eventStepCv;

    bool redraw;
};
