#pragma once
#include <optional>
#include "core/include/common.hpp"
#include "core/include/rom.hpp"
#include "log/log.hpp"

// basic class for mappers

/*
    Typical mapper memory map:
        $4020 - $5FFF - unused or some specific data;
        $6000 - $7FFF - PRG-RAM
        $8000 - $FFFF - PRG-ROM
    All values, except PRG-ROM data, can be perfectly handled by memory itself. So here we process only PRG-ROM addresses.

    I use std::optional to show if request was processed by mapper.
    If not, std::nullopt will be returned and memory should process it itself.
*/
class MapperInterface {
public:
    MapperInterface(ROM& _rom, Logger* logger=nullptr);
    virtual ~MapperInterface() {}
    virtual std::optional<u8> read8(Address);
    virtual std::optional<bool> write8(Address offset, u8 val);
    virtual std::optional<u16> read16(Address);
    virtual std::optional<bool> write16(Address offset, u16 val);
    virtual std::optional<u8> readCHR(Address);
    virtual std::optional<bool> writeCHR(Address offset, u8 val);
protected:
    virtual bool checkAddress(Address address) const = 0;
    virtual Address addressFix(Address address) const = 0;
    virtual bool checkCHRAddress(Address address) const = 0;
    virtual Address addressCHRFix(Address address) const = 0;

    ROM& rom;
    Logger* logger;
};

class InvalidMapperException {};

