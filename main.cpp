#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <SDL2/SDL.h>
#include "nes.hpp"

using namespace std;

class GuiSDL : public Observer<PPU> {
public:
    GuiSDL(u16 w, u16 h, PPU* _ppu, NES& _nes)
        : width{w}, height{h}, ppu{_ppu}, nes{_nes}, quit{false}
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
        /*if(ppu->currentFrame() == 60) {
            nes.getController(1).updateKey(StandardController::Key::Start, true);
        }*/
        redraw = true;
        //if(eventType == (int)PPUEvent::RerenderMe) render();
    }

    void render() {
        SDL_UpdateTexture(texture, NULL, &(ppu->image()[0]), width * 4);
        SDL_RenderCopy( renderer, texture, NULL, NULL );
        SDL_RenderPresent( renderer );
    }

    void eventLoop() {
        SDL_Event event;
        while(true) {
            while( SDL_PollEvent( &event ) ){
              /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
              switch( event.type ){
                case SDL_KEYDOWN: {
                  switch(event.key.keysym.sym) {
                  case SDLK_LEFT: nes.getController(0).updateKey(StandardController::Key::Left, true); break;
                  case SDLK_UP: nes.getController(0).updateKey(StandardController::Key::Up, true); break;
                  case SDLK_RIGHT: nes.getController(0).updateKey(StandardController::Key::Right, true); break;
                  case SDLK_DOWN: nes.getController(0).updateKey(StandardController::Key::Down, true); break;
                  case SDLK_RETURN: nes.getController(0).updateKey(StandardController::Key::Start, true); break;
                  case SDLK_SPACE: nes.getController(0).updateKey(StandardController::Key::Select, true); break;
                  case SDLK_z: nes.getController(0).updateKey(StandardController::Key::A, true); break;
                  case SDLK_x: nes.getController(0).updateKey(StandardController::Key::B, true); break;
                  }
                  break;
              }
                  case SDL_KEYUP: {
                    switch(event.key.keysym.sym) {
                    case SDLK_LEFT: nes.getController(0).updateKey(StandardController::Key::Left, false); break;
                    case SDLK_UP: nes.getController(0).updateKey(StandardController::Key::Up, false); break;
                    case SDLK_RIGHT: nes.getController(0).updateKey(StandardController::Key::Right, false); break;
                    case SDLK_DOWN: nes.getController(0).updateKey(StandardController::Key::Down, false); break;
                    case SDLK_RETURN: nes.getController(0).updateKey(StandardController::Key::Start, false); break;
                    case SDLK_SPACE: nes.getController(0).updateKey(StandardController::Key::Select, false); break;
                    case SDLK_z: nes.getController(0).updateKey(StandardController::Key::A, false); break;
                    case SDLK_x: nes.getController(0).updateKey(StandardController::Key::B, false); break;
                    }
                    break;
            }
                default:
                  break;
              }
            }
            if(redraw) { render(); redraw = false; }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u16 width;
    u16 height;
    PPU* ppu;
    NES& nes;

    bool redraw;
    bool quit;
};

int main()
{
    //sdl();

    std::ofstream ofs("/home/onyazuka/log.txt");
    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);
    NES nes("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Excitebike (E).nes", oslogger);

    PPU& ppu = nes.getPpu();
    GuiSDL* gui = new GuiSDL(256, 240, &ppu, nes);
    ppu.attach(gui);

    const std::string savePath = "/home/onyazuka/nesSaves/Excitebike.hns";

    nes.load(savePath);
    std::thread cpuThread([&nes, &savePath]() {
        while(true) {
            nes.doInstruction();
            /*if(nes.getCpu().getInstructionCounter() == 8600000) {
                nes.save(savePath);
                break;
            }*/
        }
    });

    gui->eventLoop();

    cpuThread.join();

    delete oslogger;
    delete gui;

    return 0;
}
