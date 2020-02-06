#pragma once
#include "core/rom.hpp"
#include "core/mappers/mapperinterface.hpp"
#include "log/log.hpp"

// MAPPER 0 (also known as NROM)

/*
    Can one or two 16kb banks.
    1 bank:
        mapped into $8000-$BFFF and mirrored on $C000 - $FFFF
    2 banks:
        mapped into $8000-$FFFF
*/
class Mapper0 : public MapperInterface {
public:
    Mapper0(ROM& _rom, Logger* logger=nullptr);
    u8 read8(Address offset);
    MapperInterface& write8(Address offset, u8 val);
    u16 read16(Address offset);
    MapperInterface& write16(Address offset, u16 val);
    bool isCorrect() const;
private:
    Address _addressFix(Address address);

    ROM& rom;
    u8 sz16kb;
    Logger* logger;
};
