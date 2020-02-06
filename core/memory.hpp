#pragma once
#include <array>
#include "common.hpp"
#include "mappers/mappers.hpp"

class Memory {
public:
    Memory(MapperInterface& _mapper);
    u8 read8(Address offset);
    Memory& write8(Address offset, u8 val);
    u16 read16(Address offset);
    Memory& write16(Address offset, u16 val);
    inline auto& get() { return memory; }
private:
    Address _mirrorAddressFix(Address address);

    std::array<u8, 0x10000> memory;
    MapperInterface& mapper;
};

bool isInPRGROM(Address address);




