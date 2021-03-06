#include "include/memory.hpp"
#include <iostream>

// - value-initializing memory(init with zeros)
Memory::Memory(MapperInterface& _mapper, PPU& _ppu, StandardController& _contr1, StandardController& _contr2)
    : memory{}, mapper{_mapper}, ppu{_ppu}, stController1{_contr1}, stController2{_contr2} {}

u8 Memory::read8(Address offset) {
    auto optionalRes = mapper.read8(offset);
    if(optionalRes) return optionalRes.value();
    offset = _mirrorAddressFix(offset);
    // ppu
    if(isInPPURegisters(offset)) {
        auto& ppuregs = ppu.accessPPURegisters();
        switch(offset) {
        case 0x2000: return ppuregs.readPpuctrl();
        case 0x2001: return ppuregs.readPpumask();
        case 0x2002: return ppuregs.readPpustatus();
        case 0x2003: return ppuregs.readOamaddr();
        case 0x2004: return ppuregs.readOamdata();
        case 0x2005: return ppuregs.readPpuscroll();
        case 0x2006: return ppuregs.readPpuaddr();
        case 0x2007: return ppuregs.readPpudata();
        case 0x4014: return ppuregs.readOamdma();
        }
    }
    // controllers
    else if (offset == 0x4016) return stController1.read();
    else if (offset == 0x4017) return stController2.read();
    // others
    return memory[offset];
}

Memory& Memory::write8(Address offset, u8 val) {
    auto optionalRes = mapper.write8(offset, val);
    if(optionalRes) return *this;
    offset = _mirrorAddressFix(offset);
    // ppu
    if(isInPPURegisters(offset)) {
        auto& ppuregs = ppu.accessPPURegisters();
        switch(offset) {
        case 0x2000: ppuregs.writePpuctrl(val); return *this;
        case 0x2001: ppuregs.writePpumask(val); return *this;
        case 0x2002: ppuregs.writePpustatus(val); return *this;
        case 0x2003: ppuregs.writeOamaddr(val); return *this;
        case 0x2004: ppuregs.writeOamdata(val); return *this;
        case 0x2005: ppuregs.writePpuscroll(val); return *this;
        case 0x2006: ppuregs.writePpuaddr(val); return *this;
        case 0x2007: ppuregs.writePpudata(val); return *this;
        case 0x4014: ppuregs.writeOamdma(val); return *this;
        }
    }
    // controllers
    else if (offset == 0x4016) {
        if (val == 0) stController1.strobe(val);
        else if (val == 1) stController2.strobe(val);
        return *this;
    }
    if(offset >= 0x6000 && offset < 0x8000) {
        int i = 0;
        i += 1;
    }
    // others
    memory[offset] = val;
    return *this;
}

u16 Memory::read16(Address offset) {
    auto optionalRes = mapper.read16(offset);
    if(optionalRes) return optionalRes.value();
    Address fixedAddress = _mirrorAddressFix(offset);
    return read16Contigous(memory, fixedAddress);
}

Memory& Memory::write16(Address offset, u16 val) {
    auto optionalRes = mapper.write16(offset, val);
    if(optionalRes) return *this;
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

bool isInPPURegisters(Address address) {
    return (address >= 0x2000 && address <= 0x2007) || (address == 0x4014);
}
