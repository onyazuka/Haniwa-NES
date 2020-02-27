#include "core/include/mappers/mapperinterface.hpp"

MapperInterface::MapperInterface(ROM& _rom, Logger* _logger)
    : rom{_rom}, logger{_logger}, _mirroring{rom.header()->mirroring()} {}

std::optional<u8> MapperInterface::read8(Address offset) {
    if(!checkAddress(offset)) return std::nullopt;
    return rom.PRGROM()[addressFix(offset)];
}

std::optional<bool> MapperInterface::write8(Address offset, u8 val) {
    if(!checkAddress(offset)) return std::nullopt;
#ifdef DEBUG
    if(logger) logger->log(LogLevel::Warning, "PRG-ROM writing attempt at " + std::to_string(offset) + " with value " + std::to_string(val));
#endif
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
#ifdef DEBUG
    if(logger) logger->log(LogLevel::Warning, "PRG-ROM writing16 attempt at " + std::to_string(offset) + " with value " + std::to_string(val));
#endif
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
#ifdef DEBUG
    if(logger) logger->log(LogLevel::Warning, "CHR-ROM writing attempt at " + std::to_string(offset) + " with value " + std::to_string(val));
#endif
    rom.CHRROM()[addressCHRFix(offset)] = val;
    return true;
}
