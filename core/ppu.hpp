#pragma once
#include <optional>
#include "common.hpp"
#include "ppumemory.hpp"

struct PPURegisters {
    PPURegisters();
    u8 ppuctrl;
    u8 ppumask;
    u8 ppustatus;
    u8 oamaddr;
    u8 oamdata;
    u8 ppuscroll;
    u8 ppuaddr;
    u8 ppudata;
    u8 oamdma;
};

class PPU;

/*
    Incapsulating ppu registers access operations.
    Also it can change ppu internal state, so we have to pass it.
*/
class PPURegistersAccess {
public:
    PPURegistersAccess(PPU& _ppu);
    inline u8 readPpuctrl() const { return ppuRegisters.ppuctrl; }
    PPURegistersAccess& writePpuctrl(u8 val);
    PPURegistersAccess& writePpuctrlNametableBase(u8 val);
    inline PPURegistersAccess& writePpuctrlVramIncrement(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 2) : clearBit(ppuRegisters.ppuctrl, 2); return *this; }
    inline PPURegistersAccess& writePpuctrlSpritePTAddr(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 3) : clearBit(ppuRegisters.ppuctrl, 3); return *this; }
    inline PPURegistersAccess& writePpuctrlBckgPTAddr(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 4) : clearBit(ppuRegisters.ppuctrl, 4); return *this; }
    inline PPURegistersAccess& writePpuctrlSpriteSize(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 5) : clearBit(ppuRegisters.ppuctrl, 5); return *this; }
    inline PPURegistersAccess& writePpuctrlMasterSlave(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 6) : clearBit(ppuRegisters.ppuctrl, 6); return *this; }
    inline PPURegistersAccess& writePpuctrlVblankNMI(u8 val) { val ? setBit(ppuRegisters.ppuctrl, 7) : clearBit(ppuRegisters.ppuctrl, 7); return *this; }
    inline u8 readPpuctrlNametableBase() const { return ppuRegisters.ppuctrl & 0b11; }
    inline u8 readPpuctrlVramIncrement() const { return (ppuRegisters.ppuctrl & 0b00000100) >> 2; }
    inline u8 readPpuctrlSpritePTAddrt() const { return (ppuRegisters.ppuctrl & 0b00001000) >> 3; }
    inline u8 readPpuctrlBckgPTAddr() const { return (ppuRegisters.ppuctrl & 0b00010000) >> 4; }
    inline u8 readPpuctrlSpriteSize() const { return (ppuRegisters.ppuctrl & 0b00100000) >> 5; }
    inline u8 readPpuctrlMasterSlave() const { return (ppuRegisters.ppuctrl & 0b01000000) >> 6; }
    inline u8 readPpuctrlVblankNMI() const { return (ppuRegisters.ppuctrl & 0b10000000) >> 7; }

    inline PPURegistersAccess& writePpumask(u8 val) { ppuRegisters.ppumask = val; return *this; }
    inline PPURegistersAccess& writePpumaskGreyscale(u8 val) { val ? setBit(ppuRegisters.ppumask, 0) : clearBit(ppuRegisters.ppumask, 0); return *this; }
    inline PPURegistersAccess& writePpumaskShowBckgLeftmost8(u8 val) { val ? setBit(ppuRegisters.ppumask, 1) : clearBit(ppuRegisters.ppumask, 1); return *this; }
    inline PPURegistersAccess& writePpumaskShowSpritesLeftmost8(u8 val) { val ? setBit(ppuRegisters.ppumask, 2) : clearBit(ppuRegisters.ppumask, 2); return *this; }
    inline PPURegistersAccess& writePpumaskShowBckg(u8 val) { val ? setBit(ppuRegisters.ppumask, 3) : clearBit(ppuRegisters.ppumask, 3); return *this; }
    inline PPURegistersAccess& writePpumaskShowSprites(u8 val) { val ? setBit(ppuRegisters.ppumask, 4) : clearBit(ppuRegisters.ppumask, 4); return *this; }
    inline PPURegistersAccess& writePpumaskEmphasizeRed(u8 val) { val ? setBit(ppuRegisters.ppumask, 5) : clearBit(ppuRegisters.ppumask, 5); return *this; }
    inline PPURegistersAccess& writePpumaskEmphasizeGreen(u8 val) { val ? setBit(ppuRegisters.ppumask, 6) : clearBit(ppuRegisters.ppumask, 6); return *this; }
    inline PPURegistersAccess& writePpumaskEmphasizeBlue(u8 val) { val ? setBit(ppuRegisters.ppumask, 7) : clearBit(ppuRegisters.ppumask, 7); return *this; }
    inline u8 readPpumask() const { return ppuRegisters.ppumask; }
    inline u8 readPpumaskGreyscale() const { return readPpumask() & 0b1; }
    inline u8 readPpumaskShowBckgLeftmost8() const { return (readPpumask() & 0b00000010) >> 1; }
    inline u8 readPpumaskShowSpritesLeftmost8() const { return (readPpumask() & 0b00000100) >> 2; }
    inline u8 readPpumaskShowBckg() const { return (readPpumask() & 0b00001000) >> 3; }
    inline u8 readPpumaskShowSprites() const { return (readPpumask() & 0b00010000) >> 4; }
    inline u8 readPpumaskEmphasizeRed() const { return (readPpumask() & 0b00100000) >> 5; }
    inline u8 readPpumaskEmphasizeGreen() const { return (readPpumask() & 0b01000000) >> 6; }
    inline u8 readPpumaskEmphasizeBlue() const { return (readPpumask() & 0b10000000) >> 7; }

    inline PPURegistersAccess& writePpustatus(u8 val) { ppuRegisters.ppustatus = val; return *this; }
    inline PPURegistersAccess& writePpustatusSpriteOverflow(u8 val) { val ? setBit(ppuRegisters.ppustatus, 5) : clearBit(ppuRegisters.ppustatus, 5); return *this; }
    inline PPURegistersAccess& writePpustatusSprite0Hit(u8 val) { val ? setBit(ppuRegisters.ppustatus, 6) : clearBit(ppuRegisters.ppustatus, 6); return *this; }
    inline PPURegistersAccess& writePpustatusVblank(u8 val) { val ? setBit(ppuRegisters.ppustatus, 7) : clearBit(ppuRegisters.ppustatus, 7); return *this; }
    u8 readPpustatus();
    inline u8 readPpustatusSpriteOverflow() { return (readPpustatus() & 0b00100000) >> 5; }
    inline u8 readPpustatusSprite0Hit() { return (readPpustatus() & 0b01000000) >> 6; }
    inline u8 readPpustatusVblank() { return (readPpustatus() & 0b10000000) >> 7; }

    inline PPURegistersAccess& writeOamaddr(u8 val) { ppuRegisters.oamaddr = val; return *this; }
    inline PPURegistersAccess& incrementOamaddr() { ppuRegisters.oamaddr++; return *this; }
    inline u8 readOamaddr() const { return ppuRegisters.oamaddr; }

    PPURegistersAccess& writeOamdata(u8 val);
    u8 readOamdata() const;

    PPURegistersAccess& writePpuscroll(u8 val);
    inline u8 readPpuscroll() const { return ppuRegisters.ppuscroll; }

    PPURegistersAccess& writePpuaddr(u8 val);
    inline u8 readPpuaddr() const { return ppuRegisters.ppuaddr; }

    PPURegistersAccess& writePpudata(u8 val);
    u8 readPpudata() const;
private:
    PPU& ppu;
    PPURegisters ppuRegisters;
};


class PPU {
public:
    // only 14 bits out of 15
    const Address AccessAddressMask = 0b11111111111111;
    friend class PPURegistersAccess;
    template<std::size_t N> using Shifts16 = std::array<u16, N>;
    template<typename std::size_t N> using Shifts8 = std::array<u8, N>;
    template<typename std::size_t N> using Latches = std::array<bool, N>;
    template<typename std::size_t N> using Counters = std::array<u8, N>;

    PPU(PPUMemory& _memory);
    std::optional<InterruptType> step();
    inline PPURegistersAccess accessPPURegisters() { return ppuRegisters; }

private:
    void preRender();
    void visibleRender();
    void postRender();
    void verticalBlank();

    void setVblank(bool val);

    PPURegistersAccess ppuRegisters;
    PPUMemory& memory;

    // --- background
    // 15 bits - current VRAM address
    // Schema: yyyNNYYYYYXXXXX (y - fine y scroll, N - nametable select(address - $2000), Y - coarse Y scroll, X - coarse X scroll)
    Address v;
    // 15 bits - temporary VRAM address(address of the top left onscreen tile)
    Address t;
    // 3 bits - fine X scroll
    u8 x;
    // 1 bit - first or second write toggle
    u8 w;
    Shifts16<2> bckgShifts16;
    Shifts8<2> bckgShifts8;

    // --- sprites
    std::array<u8, 0x100> OAM;
    std::array<u8, 0x20> secondaryOAM;
    Shifts8<16> spritesShifts8;
    Latches<8> latches;
    Counters<8> counters;

    // --- private
    u8 frame;       // cyclic is ok
    u16 scanline;
    u16 cycle;      // this scanline cycle

    bool nmiRequest;
};
