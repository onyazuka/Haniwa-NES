#include "memory.hpp"

// - value-initializing memory(init with zeros)
Memory::Memory(MapperInterface& _mapper)
    : memory{}, mapper{_mapper} {}

u8 Memory::read8(Address offset) {
    if (isInPRGROM(offset)) return mapper.read8(offset);
    Address fixedAddress = _mirrorAddressFix(offset);
    return memory[fixedAddress];
}

Memory& Memory::write8(Address offset, u8 val) {
    if (isInPRGROM(offset)) {
        mapper.write8(offset, val);
        return *this;
    };
    Address fixedAddress = _mirrorAddressFix(offset);
    memory[fixedAddress] = val;
    return *this;
}

u16 Memory::read16(Address offset) {
    if (isInPRGROM(offset)) return mapper.read16(offset);
    Address fixedAddress = _mirrorAddressFix(offset);
    return read16Contigous(memory, fixedAddress);
}

Memory& Memory::write16(Address offset, u16 val) {
    if (isInPRGROM(offset)) {
        mapper.write16(offset, val);
        return *this;
    }
    Address fixedAddress = _mirrorAddressFix(offset);
    write16Contigous(memory, fixedAddress, val);
    return *this;
}

/*
   Fix address, considering mirroring, for read/write operations.
   For example, PPU registers are stored at $2000 through $2007, and mirrored from $2008 through $3FFF, so a write to $3456 is the same as a write to $2006.
   CPU memory mirrored ranges:
    Original: $0000-$07FF, mirrors: $0800-$1FFF;
    Original: $2000-$2007, mirrors: $2008-$3FFF.
*/
Address Memory::_mirrorAddressFix(Address address) {
    if (address < 0x2000) return address % 0x0800;
    else if(address >= 0x2000 && address < 0x4000) return 0x2000 + address % 8;
    // no mirroring
    else return address;
}
