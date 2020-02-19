#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <SDL2/SDL.h>
#include "nes.hpp"

using namespace std;

class GuiSDL : public Observer<PPU> {
public:
    GuiSDL(u16 w, u16 h, PPU* _ppu)
        : width{w}, height{h}, ppu{_ppu}
    {
        SDL_Init(SDL_INIT_VIDEO);
        atexit(SDL_Quit);

        window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING, width, height);
    }

    ~GuiSDL() {
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );
        SDL_Quit();
    }

    void update(PPU*, int eventType) {
        if(eventType == (int)PPUEvent::RerenderMe) render();
    }

    void render() {
        SDL_UpdateTexture(texture, NULL, &(ppu->image()[0]), width * 4);
        SDL_RenderCopy( renderer, texture, NULL, NULL );
        SDL_RenderPresent( renderer );
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u16 width;
    u16 height;
    PPU* ppu;
};

int main()
{
    //sdl();

    std::ofstream ofs("/home/onyazuka/log.txt");
    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);
    NES nes("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Excitebike (E).nes", oslogger);

    PPU& ppu = nes.getPpu();
    /*GuiSDL* gui = new GuiSDL(256, 240, &ppu);
    ppu.attach(gui);*/

    const std::string savePath = "/home/onyazuka/nesSaves/Excitebike.hns";

    nes.load(savePath);

    while(true) {
        nes.doInstruction();
        /*if(nes.getCpu().getInstructionCounter() == 8600000) {
            nes.save(savePath);
            break;
        }*/
    }

    delete oslogger;
   // delete gui;

    return 0;
}
