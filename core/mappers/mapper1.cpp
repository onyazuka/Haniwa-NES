#include "core/include/mappers/mapper1.hpp"
#include <iostream>

Mapper1::Mapper1(ROM& _rom, Logger* logger)
    : MapperInterface(_rom, logger), rLoad{0}, rControl{0}, rChrBank0{0}, rChrBank1{0},
      rPrgBank{0}, prgBank0{0}, prgBank1{0}, prgBanks{rom.header()->PRGROMSize16Kb()},
      chrBank0{0}, chrBank1{0}
{
    initPRGBanks();
}

std::optional<bool> Mapper1::write8(Address offset, u8 val) {
    // 5 writes are needed to fill the shift register
    static u8 writeCount = 0;
    if(!checkAddress(offset)) return std::nullopt;
    // write with bit 7 set clears the shift register
    if (val & 0b10000000) {
        rLoad = 0;
        writeCount = 0;
        initPRGBanks();
    }
    else {
        // bit 0 of values becomes bit 7 of rLoad
        rLoad |= ((val & 1) << 7);
        rLoad >>= 1;
        ++writeCount;
        if(writeCount == 5) {
            // shifting so LSB of shift register's value becomes bit 0
            rLoad >>= 2;
            writeCount = 0;
            // which register to update is determined by bits 14 and 13 at the moment of 5th write
            u8 regNum = (offset & 0b110000000000000) >> 13;
            switch (regNum) {
            case 0: rControl = rLoad; break;
            case 1: rChrBank0 = rLoad; break;
            case 2: rChrBank1 = rLoad; break;
            case 3: rPrgBank = rLoad; break;
            }
            rLoad = 0;
            fix();
        }
    }
    return true;
}

// serialization
Serialization::BytesCount Mapper1::serialize(std::string &buf) {
    return Serialization::Serializer::serializeAll(buf, &rLoad, &rControl, &rChrBank0, &rChrBank1, &rPrgBank, &prgBank0, &prgBank1, &prgBanks, &chrBank0, &chrBank1);
}

Serialization::BytesCount Mapper1::deserialize(const std::string &buf, Serialization::BytesCount offset) {
    return Serialization::Deserializer::deserializeAll(buf, offset, &rLoad, &rControl, &rChrBank0, &rChrBank1, &rPrgBank, &prgBank0, &prgBank1, &prgBanks, &chrBank0, &chrBank1);
}

bool Mapper1::checkAddress(Address address) const {
    // if PRG RAM is used, read/write should be processed by 'memory'
    return address >= 0x8000;
}

Address Mapper1::addressFix(Address address) const {
    bool usingBank0 = address < 0xC000;
    if (usingBank0) return (prgBank0 * PRG_BANK_SIZE) + (address - 0x8000);
    else return (prgBank1 * PRG_BANK_SIZE) + (address - 0xC000);
}

bool Mapper1::checkCHRAddress(Address address) const {
    return address < 0x2000;
}

Address Mapper1::addressCHRFix(Address address) const {
    bool usingBank0 = address < 0x1000;
    if (usingBank0) return (chrBank0 * CHR_BANK_SIZE) + address;
    else return (chrBank1 * CHR_BANK_SIZE) + (address - 0x1000);
}

// initial mode is mode 3 (?)
void Mapper1::initPRGBanks() {
    rControl |= 0x0c;
    fixPRGBanks();
}

void Mapper1::fix() {
    fixMirroring();
    fixCHRBanks();
    fixPRGBanks();
}

void Mapper1::fixPRGBanks() {
    u8 prgBankMode = getPrgRomBankMode();
    if((prgBankMode == 0 || prgBankMode) == 1 && (rPrgBank & 0b10000)) logger->log(LogLevel::Warning, "Attempt to select PRG ROM bank with bit 4");
    switch(prgBankMode & 0b11) {
    // 0, 1: switch 32 KB at $8000, ignoring low bit of bank number
    case 0:
    case 1:
        prgBank0 = rPrgBank & 0b01110;
        prgBank1 = prgBank0 + 1;
        break;
    case 2:
        prgBank0 = 0;
        prgBank1 = rPrgBank & 0b1111;
        break;
    case 3:
        prgBank0 = rPrgBank & 0b1111;
        prgBank1 = prgBanks - 1;
        break;
    }
}

void Mapper1::fixCHRBanks() {
    bool switchChr8Kb = !(rControl & 0b10000);
    // switching 8 kb at once
    if (switchChr8Kb) {
        chrBank0 = rChrBank0 & 0b11110;
        chrBank1 = chrBank0 + 1;
    }
    // switching 2 separate CHR banks
    else {
        chrBank0 = rChrBank0;
        chrBank1 = rChrBank1;
    }
}

void Mapper1::fixMirroring() {
    switch(rControl & 0b11) {
    case 0: _mirroring = Mirroring::OneScreenLower; break;
    case 1: _mirroring = Mirroring::OneScreenUpper; break;
    case 2: _mirroring = Mirroring::Vertical; break;
    case 3: _mirroring = Mirroring::Horizontal; break;
    }
}
