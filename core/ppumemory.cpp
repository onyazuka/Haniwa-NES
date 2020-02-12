#include "ppumemory.hpp"

PPUMemory::PPUMemory(MapperInterface &_mapper, Mirroring _mirroring, Logger* _logger)
    : memory{}, mapper{_mapper}, mirroring{_mirroring}, logger{_logger} {}

u8 PPUMemory::read(Address address) {
    auto optionalRes = mapper.readCHR(address);
    if (optionalRes) return optionalRes.value();
    address = _fixAddress(address);
    return memory[address];
}

PPUMemory& PPUMemory::write(Address address, u8 val) {
    auto optionalRes = mapper.writeCHR(address, val);
    if (optionalRes) return *this;
    address = _fixAddress(address);
    memory[address] = val;
    return *this;
}

Address PPUMemory::_fixAddress(Address address) {
    if(address >= 0x2000 && address < 0x3000) return _applyMirroring(address);
    if(address >= 0x3000 && address < 0x3F00) return address - 0x1000;      // mirroring nametables
    if(address >= 0x3F20 && address < 0x4000) return 0x3F00 + (address % 0x20); // mirroring pallette indexes
    if(address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || 0x3F1C) return address - 0x0010;  // also palette mirroring
    return address;
}

Address PPUMemory::_applyMirroring(Address address) {
    switch (mirroring) {
    case Mirroring::Horizontal: if (address & 0x400) return address - 0x400; else return address;
    case Mirroring::Vertical: if(address & 0x800) return address - 0x800; else return address;
    default: if(logger) logger->log(LogLevel::Error, "PPUMemory::_applyMirroring() - unknown mirroring type");  throw UnknownMirroringType{};
    }
}
