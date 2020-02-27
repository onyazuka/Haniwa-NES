#pragma once
#include "core/include/rom.hpp"
#include "core/include/mappers/mapperinterface.hpp"
#include "log/log.hpp"

// MAPPER 0 (also known as NROM)

/*
    Can have one 32kb or two 16kb banks.
    1 bank:
        mapped into $8000-$BFFF and mirrored on $C000 - $FFFF
    2 banks:
        mapped into $8000-$FFFF
*/
class Mapper0 : public MapperInterface {
public:
    Mapper0(ROM& _rom, Logger* logger=nullptr);
    bool isCorrect() const;
private:
    bool checkAddress(Address address) const;
    Address addressFix(Address address) const;
    bool checkCHRAddress(Address address) const;
    Address addressCHRFix(Address address) const;

    u8 sz16kb;
};
