#include "ppu.hpp"
#include <cstring>

PPURegisters::PPURegisters()
    : ppuctrl{0}, ppumask{0}, ppustatus{0}, oamaddr{0}, ppuscroll{0}, ppuaddr{0}, ppudata{0} {}

PPURegisters::PPUCTRL::PPUCTRL(u8 val) {
    nametableBase = val & 0b11;
    vramIncrement = (val >> 2) & 1;
    patternTable = (val >> 3) & 1;
    bckgTable = (val >> 4) & 1;
    spriteSize = (val >> 5) & 1;
    masterSlave = (val >> 6) & 1;
    vblankNMI = (val >> 7) & 1;
}

PPURegisters::PPUMASK::PPUMASK(u8 val) {
    greyscale = val & 1;
    showBckgLeftmost8 = (val >> 1) & 1;
    showSpritesLeftmost8 = (val >> 2) & 1;
    showBckg = (val >> 3) & 1;
    showSprites = (val >> 4) & 1;
    emphasizeRed = (val >> 5) & 1;
    emphasizeGreen = (val >> 6) & 1;
    emphasizeBlue = (val >> 7) & 1;
}

PPURegisters::PPUSTATUS::PPUSTATUS(u8 val) {
    spriteOverflow = (val >> 5) & 1;
    sprite0Hit = (val >> 6) & 1;
    vblank = (val >> 7) & 1;
}

PPU::PPU()
    : v{0}, t{0}, x{0}, w{0}, OAM{}, secondaryOAM{}, spritesShifts8{}, latches{}, counters{},
      frame{0}, scanline{0}, cycle{0} {}

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
    ppuRegisters.ppustatus.vblank = 1;
    // nmi request will be send after step is complete
    if(ppuRegisters.ppuctrl.vblankNMI) nmiRequest = true;
}
