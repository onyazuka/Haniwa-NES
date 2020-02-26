#pragma once
#include <array>
#include "ppu.hpp"
#include "input.hpp"
#include "common.hpp"
#include "mappers/mappers.hpp"

class Memory {
public:
    Memory(MapperInterface& _mapper, PPU& _ppu, StandardController& _contr1, StandardController& _contr2);
    u8 read8(Address offset);
    Memory& write8(Address offset, u8 val);
    u16 read16(Address offset);
    Memory& write16(Address offset, u16 val);
    inline auto& get() { return memory; }
private:
    Address _mirrorAddressFix(Address address);

    std::array<u8, 0x10000> memory;
    MapperInterface& mapper;
    // needed to access ppu registers
    PPU& ppu;
    StandardController& stController1;
    StandardController& stController2;
};

bool isInPPURegisters(Address address);




