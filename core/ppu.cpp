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
