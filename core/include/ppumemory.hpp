#pragma once
#include <array>
#include "core/include/common.hpp"
#include "core/include/mappers/mappers.hpp"

const std::array<u32, 64> Palette = {
    4605510, 1626, 1656, 132723, 3474252, 5701646, 5898240, 4259840, 1180160, 5120, 7680, 7680, 5409, 0, 0, 0, 10329501, 19129,
    340193, 5707994, 10422183, 13369941, 13568768, 10756864, 6045440, 743424, 26112, 26387, 24174, 0, 0, 0, 16711679, 2072319,
    5469951, 9987583, 16541695, 16739507, 16741478, 16744468, 12884480, 7451392, 2671649, 51316, 49104, 2829099, 0, 0, 16711679,
    10409471, 11518207, 13678847, 16695295, 16761056, 16761789, 16763548, 15193483, 12967822, 10938019, 9758917, 9626859, 10987431, 0, 0
};


class PPUMemory {
public:
    PPUMemory(MapperInterface& _mapper, Logger* logger=nullptr);
    u8 read(Address address);
    // USE WITH CARE!
    u8 readCHR(Address address);
    u8 readDirectly(Address address);
    u8 readDirectlyWithoutChecks(Address address);
    PPUMemory& write(Address address, u8 val);
    inline auto& getMemory () { return memory; }
private:
    Address _fixAddress(Address address);
    Address _applyMirroring(Address address);

    std::array<u8, 0x4000> memory;
    MapperInterface& mapper;
    Logger* logger;
};
