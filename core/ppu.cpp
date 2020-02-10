#include "ppu.hpp"
#include <cstring>

PPURegisters::PPURegisters()
    : ppuctrl{0}, ppumask{0}, ppustatus{0}, oamaddr{0}, ppuscroll{0}, ppuaddr{0}, ppudata{0} {}

PPURegistersAccess::PPURegistersAccess(PPU &_ppu)
    : ppu{_ppu} {}

PPURegistersAccess& PPURegistersAccess::writePpuctrl(u8 val) {
    // writing base nametable address to ppu's register t
    ppuRegisters.ppuctrl = val;
    ppu.t |= (val & 0b11) << 10;
    return *this;
}

PPURegistersAccess& PPURegistersAccess::writePpuctrlNametableBase(u8 val) {
    (val & 1) ? setBit(ppuRegisters.ppuctrl, 0) : clearBit(ppuRegisters.ppuctrl, 0);
    (val & 2) ? setBit(ppuRegisters.ppuctrl, 1) : clearBit(ppuRegisters.ppuctrl, 1);
    ppu.t |= (val) << 10;
    return *this;
}

u8 PPURegistersAccess::readPpustatus() {
    ppu.w = 0;
    clearBit(ppuRegisters.ppustatus, 7);
    return ppuRegisters.ppustatus;
}

PPURegistersAccess& PPURegistersAccess::writeOamdata(u8 val) {
    ppu.OAM[ppuRegisters.oamaddr] = val;
    incrementOamaddr();
    return *this;
}

u8 PPURegistersAccess::readOamdata() const {
    return ppu.OAM[ppuRegisters.oamaddr];
}

PPURegistersAccess& PPURegistersAccess::writePpuscroll(u8 val) {
    if(ppu.w == 0) {
        // setting coarse scroll
        ppu.t ^= (ppu.t & 0b11111);
        ppu.t |= (val & 0b11111000) >> 3;
        // setting fine scroll
        ppu.x = val & 0b111;
        // setting that first write has occured
        ppu.w = 1;
    }
    // ppu.w == 1
    else {
        // setting y fine and y coarse scroll
        ppu.t ^= (ppu.t & 0b111001111100000);
        ppu.t |= (Address)(val & 0b111) << 12;
        ppu.t |= (Address)(val & 0b11111000) << 2;
        ppu.w = 0;
    }
    return *this;
}

PPURegistersAccess& PPURegistersAccess::writePpuaddr(u8 val) {
    if (ppu.w == 0) {
        // resetting both most significant bytes of address and bit X - 15th bit that should be 0 after this write
        ppu.t ^= (ppu.t & 0b111111100000000);
        ppu.t |= (Address)(val & 0b00111111) << 8;
        ppu.w = 1;
    }
    // ppu.w == 1
    else {
        ppu.t ^= (ppu.t & 0b11111111);
        ppu.t |= val;
        // filling ppu's internal address register from temporary address register
        ppu.v = ppu.t;
        ppu.w = 0;
    }
    return *this;
}

PPURegistersAccess& PPURegistersAccess::writePpudata(u8 val) {
    ppu.memory.write(ppu.v & ppu.AccessAddressMask, val);
    // if ppuctrl 2nd bit is set, incrementing by 32, else by 1
    ppu.v += readPpuctrlVramIncrement() ? 32 : 1;
    return *this;
}

u8 PPURegistersAccess::readPpudata() const {

}

PPU::PPU(PPUMemory& _memory)
    : ppuRegisters{*this}, memory{_memory}, v{0}, t{0}, x{0}, w{0}, OAM{}, secondaryOAM{}, spritesShifts8{},
      latches{}, counters{}, frame{0}, scanline{0}, cycle{0} {}

// if some interrupt occure, it will be returned
std::optional<InterruptType> PPU::step() {
    switch(scanline) {
        case 0: case 262: preRender(); break;
        case 1 ... 240: visibleRender(); break;
        case 241: postRender(); break;
        case 242 ... 261: verticalBlank(); break;
    }
    // each scanline consists of exactly 341 cycles
    cycle = (cycle + 1) % 341;
    if (cycle == 0) ++scanline;
    // odd frames skip 262 prerender, even - don't skip
    if (((scanline == 262) && (frame % 2)) || ((scanline == 263 ) && !(frame % 2))) {
        ++frame;
        scanline = 0;
    }

    if (nmiRequest) {
        nmiRequest = false;
        return InterruptType::NMI;
    }
    return std::nullopt;
}

void PPU::preRender() {

}

void PPU::visibleRender() {

}

// nice stage, just idling here
void PPU::postRender() {
    return;
}

// just turning on vblank on cycle number 1(SECOND cycle)
void PPU::verticalBlank() {
    ppuRegisters.writePpustatusVblank(1);
    // nmi request will be send after step is complete
    if(ppuRegisters.readPpuctrlVblankNMI()) nmiRequest = true;
}
