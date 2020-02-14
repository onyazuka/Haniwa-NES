#include <iostream>
#include <cstring>
#include <thread>
#include <SDL2/SDL.h>
#include "core/cpu.hpp"
#include "core/ppu.hpp"
#include "core/rom.hpp"


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

/*void sdl() {
    SDL_Init( SDL_INIT_EVERYTHING );
    atexit( SDL_Quit );

    SDL_Window* window = SDL_CreateWindow
        (
        "SDL2",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        600, 600,
        SDL_WINDOW_SHOWN
        );

    SDL_Renderer* renderer = SDL_CreateRenderer
        (
        window,
        -1,
        SDL_RENDERER_ACCELERATED
        );

    const unsigned int texWidth = 1024;
    const unsigned int texHeight = 1024;
    SDL_Texture* texture = SDL_CreateTexture
        (
        renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        texWidth, texHeight
        );

    vector< u32 > pixels( texWidth * texHeight, 0 );

    for(int i = 0; i < 10000; ++i) {

        for(int i = 0; i < texWidth; ++i) {
            for(int j = 0; j < texHeight; ++j) {
                u32 a = 255;
                u32 r = rand() % 256;
                u32 g = rand() % 256;
                u32 b = rand() % 256;
                pixels[i * texWidth + j] = (b << 24) + (g << 16) + (r << 8) + a;
            }
        }

        //unsigned char* lockedPixels;
        //int pitch;
        //SDL_LockTexture
        //    (
        //    texture,
        //    NULL,
        //    reinterpret_cast< void** >( &lockedPixels ),
        //    &pitch
        //    );
        //std::copy( pixels.begin(), pixels.end(), lockedPixels );
        //SDL_UnlockTexture( texture );

        SDL_UpdateTexture
            (
            texture,
            NULL,
            &pixels[0],
            texWidth * 4
            );

        SDL_RenderCopy( renderer, texture, NULL, NULL );
        SDL_RenderPresent( renderer );

    }


    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}*/

int main()
{
    std::cout << (int)ROL((u8)210, 1) << std::endl;
    std::cout << (int)ROL((u8)210, 3) << std::endl;
    std::cout << (int)ROR((u8)210, 1) << std::endl;
    std::cout << (int)ROR((u8)210, 3) << std::endl;

    std::cout << numToHexStr(254, 4) << std::endl;

    std::cout << Palette[15] << std::endl;
    std::cout << Palette[44] << std::endl;
    std::cout << Palette[56] << std::endl;
    std::cout << Palette[18] << std::endl;

    struct s {
        u8 n1 : 1;
        u8 n2 : 1;
        u8 n3 : 1;
        u8 n4 : 1;
        u8 n5 : 1;
        u8 n6 : 1;
        u8 n7 : 1;
        u8 n8 : 1;
    } ss;
    memset(&ss, 0b11000000, 1);

    //sdl();

    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);
    ROM rom("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Donkey_Kong.nes", oslogger);
    Mapper0 mapper(rom, oslogger);
    PPUMemory ppuMemory{mapper, rom.header()->mirroring(), oslogger};
    EventQueue eventQueue;
    PPU ppu{ppuMemory, eventQueue, oslogger};
    Memory memory(mapper, ppu);
    CPU cpu(memory, ppu, eventQueue, oslogger);

    GuiSDL* gui = new GuiSDL(256, 240, &ppu);
    ppu.attach(gui);

    cpu.run();

    delete oslogger;
    delete gui;

    return 0;
}
