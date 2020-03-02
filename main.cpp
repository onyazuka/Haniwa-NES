#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>

#include "nes.hpp"
#include "gui/sdlgui.hpp"

int main()
{
    //sdl();

    std::ofstream ofs("/home/onyazuka/log.txt");
    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);
    NES nes("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Super Mario Bros./Super Mario Bros. (W) [!].nes", oslogger);

    PPU& ppu = nes.getPpu();
    GuiSDL* gui = new GuiSDL(256, 240, &ppu, nes);
    ppu.attach(gui);

    const std::string savePath = "/home/onyazuka/nesSaves/Mario.hns";

    //nes.load(savePath);
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
