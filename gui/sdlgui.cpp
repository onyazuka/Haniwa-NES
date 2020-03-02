#include "gui/sdlgui.hpp"

GuiSDL::GuiSDL(u16 w, u16 h, PPU* _ppu, NES& _nes)
    : width{w}, height{h}, ppu{_ppu}, nes{_nes}
{
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    window = SDL_CreateWindow("HaniwaNES", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,SDL_TEXTUREACCESS_STREAMING, width, height);
}

GuiSDL::~GuiSDL() {
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();
}

void GuiSDL::update(PPU*, int) {
    redraw = true;
    eventStepCv.notify_one();
}

void GuiSDL::eventLoop() {
    std::unique_lock<std::mutex> lck{redrawMtx};
    while(true) {
        while (!redraw) eventStepCv.wait(lck);
        redraw = false;
        render();
        fetchEvents();
    }
}

void GuiSDL::render() {
    SDL_UpdateTexture(texture, NULL, &(ppu->publicImage()[0]), width * 4);
    SDL_RenderCopy( renderer, texture, NULL, NULL );
    SDL_RenderPresent( renderer );
}

void GuiSDL::fetchEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
      switch (event.type) {
        case SDL_KEYDOWN: {
          // using scancodes(but I don't really know is it right), because if use keysym.sym, code becomes locale dependant
          switch(event.key.keysym.scancode) {
          case SDL_SCANCODE_LEFT: nes.getController(0).updateKey(StandardController::Key::Left, true); break;
          case SDL_SCANCODE_UP: nes.getController(0).updateKey(StandardController::Key::Up, true); break;
          case SDL_SCANCODE_RIGHT: nes.getController(0).updateKey(StandardController::Key::Right, true); break;
          case SDL_SCANCODE_DOWN: nes.getController(0).updateKey(StandardController::Key::Down, true); break;
          case SDL_SCANCODE_RETURN: nes.getController(0).updateKey(StandardController::Key::Start, true); break;
          case SDL_SCANCODE_SPACE: nes.getController(0).updateKey(StandardController::Key::Select, true); break;
          case SDL_SCANCODE_Z: nes.getController(0).updateKey(StandardController::Key::A, true); break;
          case SDL_SCANCODE_X: nes.getController(0).updateKey(StandardController::Key::B, true); break;
          default: break;
          }
          break;
      }
         case SDL_KEYUP: {
           switch(event.key.keysym.scancode) {
           case SDL_SCANCODE_LEFT: nes.getController(0).updateKey(StandardController::Key::Left, false); break;
           case SDL_SCANCODE_UP: nes.getController(0).updateKey(StandardController::Key::Up, false); break;
           case SDL_SCANCODE_RIGHT: nes.getController(0).updateKey(StandardController::Key::Right, false); break;
           case SDL_SCANCODE_DOWN: nes.getController(0).updateKey(StandardController::Key::Down, false); break;
           case SDL_SCANCODE_RETURN: nes.getController(0).updateKey(StandardController::Key::Start, false); break;
           case SDL_SCANCODE_SPACE: nes.getController(0).updateKey(StandardController::Key::Select, false); break;
           case SDL_SCANCODE_Z: nes.getController(0).updateKey(StandardController::Key::A, false); break;
           case SDL_SCANCODE_X: nes.getController(0).updateKey(StandardController::Key::B, false); break;
           default: break;
           }
           break;
       }
       default:
         break;
    }
  }
}
