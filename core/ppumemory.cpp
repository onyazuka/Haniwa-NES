#include "ppumemory.hpp"

PPUMemory::PPUMemory(MapperInterface &_mapper)
    : memory{}, mapper{_mapper} {}

u8 PPUMemory::read(Address address) {
    address = _fixAddress(address);
    return memory[address];
}

PPUMemory& PPUMemory::write(Address address, u8 val) {
    address = _fixAddress(address);
    memory[address] = val;
    return *this;
}

Address PPUMemory::_fixAddress(Address address) {
    if(address >= 0x3000 && address < 0x3F00) return address - 0x1000;      // mirroring nametables
    if(address >= 0x3F20 && address < 0x4000) return 0x3F00 + (address % 0x20); // mirroring pallette indexes
    return address;
}
