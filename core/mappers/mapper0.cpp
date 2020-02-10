#include "core/mappers/mapper0.hpp"

Mapper0::Mapper0(ROM& _rom, Logger* _logger)
    : MapperInterface{_rom, _logger}, sz16kb{rom.header()->PRGROMSize16Kb()} {
    if(!isCorrect()) {
        if (logger) logger->log(LogLevel::Error, "Mapper 0 should have 1 or 2 PRG ROM banks");
        throw InvalidMapperException{};
    }
}

bool Mapper0::isCorrect() const {
    return sz16kb == 1 || sz16kb == 2;
}

bool Mapper0::checkAddress(Address address) const {
    return address >= 0x8000;
}

Address Mapper0::addressFix(Address address) const {
    if(address < 0x8000) if(logger) logger->log(LogLevel::Warning, "Attempt to access mapper0 with incorrect address " + std::to_string(address));
    if(sz16kb == 1) return address % 0x4000;
    else return address;        // 2
}

bool Mapper0::checkCHRAddress(Address address) const {
    return address < 0x2000;
}

Address Mapper0::addressCHRFix(Address address) const {
    return address;
}