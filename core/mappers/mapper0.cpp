#include "core/mappers/mapper0.hpp"

Mapper0::Mapper0(ROM& _rom, Logger* _logger)
    : rom{_rom}, sz16kb{rom.header()->PRGROMSize16Kb()}, logger{_logger} {
    if(!isCorrect()) {
        if (logger) logger->log(LogLevel::Error, "Mapper 0 should have 1 or 2 PRG ROM banks");
        throw InvalidMapperException{};
    }
}

u8 Mapper0::read8(Address offset) {
    return rom.PRGROM()[_addressFix(offset)];
}

MapperInterface& Mapper0::write8(Address offset, u8 val) {
    rom.PRGROM()[_addressFix(offset)] = val;
    return *this;
}

u16 Mapper0::read16(Address offset) {
    Address fixedAddress = _addressFix(offset);
    return read16Contigous(rom.PRGROM(), fixedAddress);
}

MapperInterface& Mapper0::write16(Address offset, u16 val) {
    Address fixedAddress = _addressFix(offset);
    write16Contigous(rom.PRGROM(), fixedAddress, val);
    return *this;
}

bool Mapper0::isCorrect() const {
    return sz16kb == 1 || sz16kb == 2;
}

Address Mapper0::_addressFix(Address address) {
    if(address < 0x8000) if(logger) logger->log(LogLevel::Warning, "Attempt to access to mapper0 with incorrect address " + std::to_string(address));
    if(sz16kb == 1) return address % 0x4000;
    else return address;        // 2
}
