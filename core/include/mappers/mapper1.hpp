#pragma once
#include "core/include/rom.hpp"
#include "core/include/mappers/mapperinterface.hpp"
#include "log/log.hpp"
#include "serialize/serializer.hpp"

// MAPPER 1 (MMC1)

class Mapper1 : public MapperInterface {
public:
    Mapper1(ROM& _rom, Logger* logger=nullptr);
    std::optional<bool> write8(Address offset, u8 val);
    inline u8 getPrgRomBankMode() const { return (rControl & 0b1100) >> 2; }
    // serialization
    Serialization::BytesCount serialize(std::string &buf);
    Serialization::BytesCount deserialize(const std::string &buf, Serialization::BytesCount offset);
private:
    bool checkAddress(Address address) const;
    Address addressFix(Address address) const;
    bool checkCHRAddress(Address address) const;
    Address addressCHRFix(Address address) const; 

    void initPRGBanks();
    void fix();
    void fixPRGBanks();
    void fixCHRBanks();
    void fixMirroring();

    // registers
    u8 rLoad;
    u8 rControl;
    u8 rChrBank0;
    u8 rChrBank1;
    u8 rPrgBank;

    // banks
    u8 prgBank0;
    u8 prgBank1;
    u8 prgBanks;
    u8 chrBank0;
    u8 chrBank1;
};
