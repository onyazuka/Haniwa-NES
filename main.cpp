#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <QApplication>

#include "nes.hpp"
#include "gui/sdlgui.hpp"
#include "gui/neswindow.hpp"

int main(int argc, char *argv[])
{
    OstreamLogger* oslogger = new OstreamLogger(std::cout, 0b1110);

    QApplication a(argc, argv);
    NESWindow mw(oslogger);
    mw.show();

    // if I init sdl from Qt widget(main window), it won't start
    GuiSDL* gui = new GuiSDL(256, 240, reinterpret_cast<void*>(mw.getRenderWidget()->winId()));
    mw.setRenderer(gui);

    //mw.loadRom("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Super Mario Bros./Super Mario Bros. (W) [!].nes");

    a.exec();

    delete oslogger;
    delete gui;

    return 0;
}
