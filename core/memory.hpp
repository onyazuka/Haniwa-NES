#pragma once
#include <array>
#include "common.hpp"

class Memory {
public:
    Memory();
    inline u8 read8(Address offset) { return memory[offset]; }
    inline Memory& write8(Address offset, u8 val) { memory[offset] = val; return *this; }
    inline u16 read16(Address offset) { return memory[offset] + (memory[offset + 1] << 8); }
    inline Memory& write16(Address offset, u16 val) { memory[offset] = (u8)(val & 0xff); memory[offset + 1] = (u8)((val & 0xff00) >> 8);  return *this; }
private:
    std::array<u8, 0x10000> memory;
};


