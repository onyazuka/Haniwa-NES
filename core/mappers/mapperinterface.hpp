#pragma once
#include "core/common.hpp"

// basic class for mappers

/*
    Typical mapper memory map:
        $4020 - $5FFF - unused or some specific data;
        $6000 - $7FFF - PRG-RAM
        $8000 - $FFFF - PRG-ROM
    All values, except PRG-ROM data, can be perfectly handled by memory itself. So here we process only PRG-ROM addresses.
*/

class MapperInterface {
public:
    virtual u8 read8(Address) = 0;
    virtual MapperInterface& write8(Address offset, u8 val) = 0;
    virtual u16 read16(Address) = 0;
    virtual MapperInterface& write16(Address offset, u16 val) = 0;
    virtual bool isCorrect() const = 0;
};

class InvalidMapperException {};
