#pragma once
#include <optional>
#include "common.hpp"

struct PPURegisters {
    PPURegisters();
    struct PPUCTRL {
        PPUCTRL(u8 val);
        u8 nametableBase;
        u8 vramIncrement;
        u8 patternTable;
        u8 bckgTable;
        u8 spriteSize;
        u8 masterSlave;
        u8 vblankNMI;
    } ppuctrl;

    struct PPUMASK {
        PPUMASK(u8 val);
        u8 greyscale;
        u8 showBckgLeftmost8;
        u8 showSpritesLeftmost8;
        u8 showBckg;
        u8 showSprites;
        u8 emphasizeRed;
        u8 emphasizeGreen;
        u8 emphasizeBlue;
    } ppumask;

    struct PPUSTATUS {
        PPUSTATUS(u8 val);
        u8 spriteOverflow;
        u8 sprite0Hit;
        u8 vblank;
    } ppustatus;

    u8 oamaddr;
    u8 oamdata;
    u8 ppuscroll;
    u8 ppuaddr;
    u8 ppudata;
    u8 oamdma;
};



class PPU {
public:
    template<std::size_t N> using Shifts16 = std::array<u16, N>;
    template<typename std::size_t N> using Shifts8 = std::array<u8, N>;
    template<typename std::size_t N> using Latches = std::array<bool, N>;
    template<typename std::size_t N> using Counters = std::array<u8, N>;

    PPU();
    std::optional<InterruptType> step();
private:
    void preRender();
    void visibleRender();
    void postRender();
    void verticalBlank();

    void setVblank(bool val);

    PPURegisters ppuRegisters;
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
