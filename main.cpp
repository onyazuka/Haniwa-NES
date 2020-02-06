#include <iostream>
#include <cstring>
#include "core/cpu.hpp"
#include "core/rom.hpp"

using namespace std;

int main()
{
    std::cout << (int)ROL((u8)210, 1) << std::endl;
    std::cout << (int)ROL((u8)210, 3) << std::endl;
    std::cout << (int)ROR((u8)210, 1) << std::endl;
    std::cout << (int)ROR((u8)210, 3) << std::endl;

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

    OstreamLogger* oslogger = new OstreamLogger(std::cout);
    ROM rom("/home/onyazuka/cpp/ProjectsMy/HaniwaNES/roms/Ice Climber (U) .nes", oslogger);
    delete oslogger;

    return 0;
}
