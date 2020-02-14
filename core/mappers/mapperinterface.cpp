#include "mapperinterface.hpp"

MapperInterface::MapperInterface(ROM& _rom, Logger* _logger)
    : rom{_rom}, logger{_logger} {}

std::optional<u8> MapperInterface::read8(Address offset) {
    if(!checkAddress(offset)) return std::nullopt;
    return rom.PRGROM()[addressFix(offset)];
}

std::optional<bool> MapperInterface::write8(Address offset, u8 val) {
    if(!checkAddress(offset)) return std::nullopt;
    rom.PRGROM()[addressFix(offset)] = val;
    return true;
}

std::optional<u16> MapperInterface::read16(Address offset) {
    if(!checkAddress(offset)) return std::nullopt;
    Address fixedAddress = addressFix(offset);
    return read16Contigous(rom.PRGROM(), fixedAddress);
}

std::optional<bool> MapperInterface::write16(Address offset, u16 val) {
    if(!checkAddress(offset)) return std::nullopt;
    Address fixedAddress = addressFix(offset);
    write16Contigous(rom.PRGROM(), fixedAddress, val);
    return true;
}

std::optional<u8> MapperInterface::readCHR(Address offset) {
    if(!checkCHRAddress(offset)) return std::nullopt;
    return rom.CHRROM()[addressCHRFix(offset)];
}

std::optional<bool> MapperInterface::writeCHR(Address offset, u8 val) {
    if(!checkCHRAddress(offset)) return std::nullopt;
    if(logger) logger->log(LogLevel::Warning, "CHR-ROM writing attempt at " + std::to_string(offset) + " with value " + std::to_string(val));
    rom.CHRROM()[addressCHRFix(offset)] = val;
    return true;
}
