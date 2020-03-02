#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <QApplication>

#include "nes.hpp"
#include "gui/sdlgui.hpp"
#include "gui/qtmainwindow.hpp"

int main(int argc, char *argv[])
{
    std::ofstream ofs("/home/onyazuka/log.txt");
    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);
    NES nes("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Super Mario Bros./Super Mario Bros. (W) [!].nes", oslogger);

    QApplication a(argc, argv);
    QtMainWindow mw(nes);
    mw.show();

    PPU& ppu = nes.getPpu();
    GuiSDL* gui = new GuiSDL(256, 240, &ppu, nes, reinterpret_cast<void*>(mw.getRenderWidget()->winId()));
    mw.setRenderer(gui);
    ppu.attach(&mw);

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

    a.exec();
    //gui->eventLoop();

    cpuThread.join();

    delete oslogger;
    delete gui;

    return 0;
}
