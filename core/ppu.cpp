#include <cstring>
#include "ppu.hpp"

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
    u8 res = ppu.memory.read(ppu.v & ppu.AccessAddressMask);
    ppu.v += readPpuctrlVramIncrement() ? 32 : 1;
    return res;
}

PPURegistersAccess& PPURegistersAccess::writeOamdma(u8 val) {
    ppuRegisters.oamdma = val;
    ppu.eventQueue.get().push(EventType::OAMDMAWrite);
    return *this;
}

PPU::PPU(PPUMemory& _memory, EventQueue& _eventQueue, Logger* _logger)
    : ppuRegisters{*this}, memory{_memory}, eventQueue{_eventQueue}, logger{_logger}, v{0}, t{0}, x{0}, w{0}, OAM{},
      secondaryOAM{}, spritesShifts8{}, latches{}, counters{}, frame{0}, scanline{-1}, cycle{0}, _image{} {}

void PPU::step() {
    switch(scanline) {
        case -1: preRender(); break;
        case 0 ... 239: visibleRender(); pixelRender(); break;
        case 240: postRender(); break;
        case 241 ... 260: verticalBlank(); break;
    }
    // each scanline consists of exactly 341 cycles
    cycle = (cycle + 1) % 341;
    if (cycle == 0) ++scanline;
    // odd frames skip 262 prerender, even - don't skip
    if ((scanline == -1) && (frame % 2) && (cycle == 340)) {
        cycle = 0;
        ++scanline;
    }
    if (scanline == 261) {
        scanline = 0;
        cycle = 0;
        ++frame;
    }
}

void PPU::drawPixel() {

}

void PPU::preRender() {
    // first cycle is idle
    if (cycle == 0) return;
    // clear: Vblank, sprite0, overflow
    else if (cycle == 1) {
        ppuRegisters.writePpustatusVblank(0).writePpustatusSprite0Hit(0).writePpustatusSpriteOverflow(0);
    }
    // fetching pixel data (it is unnecessary on pre render stage, but it happens anyways)
    if (cycle >= 1 && cycle <= 256) {
        _renderInternalFetchByte();
        // because it is pre render stage, we are not doing anymore here(while in renter stage we should fill registers and make a pixel)
    }
    // hor(v) = hor(t) - restoring horizontal scroll
    else if (cycle == 257) _restoreXScrollFromT();
    // vert(v) = vert(t)
    else if (cycle >= 280 && cycle <= 304)  _restoreYScrollFromT();
    else if (cycle >= 321 && cycle <= 336) {
        _renderInternalFetchByte();
    }
    // updating shifters
    if(cycle == 329 || cycle == 337) {
        _renderInternalFedToShiftRegisters();
    }
    // cycle 340 can be skipped(in 'step' function)
    // unknown NT fetches
    if(cycle >= 337 && cycle <= 340) {
        _renderInternalUnknownNTFetches();
    }
}

void PPU::visibleRender() {
    switch(cycle) {
    case 0: return;
    case 1 ... 256:
        _renderInternalFetchByte();
        drawPixel();
        break;
    case 257:
        _restoreXScrollFromT();
        // DON'T NEED BREAK HERE
    case 258 ... 320:

        break;
    case 321 ... 336:
        _renderInternalFetchByte();
        drawPixel();
        break;
    case 337 ... 340:
        _renderInternalUnknownNTFetches();
        break;
    }
    if(((cycle >= 1) && (cycle <= 257) && ((cycle - 1 % 8) == 0)) || (cycle == 329 || cycle == 337) ) {
        _renderInternalFedToShiftRegisters();
    }
}

// nice stage, just idling here
void PPU::postRender() {
    return;
}

// just turning on vblank on cycle number 1(SECOND cycle)
void PPU::verticalBlank() {
    if (cycle == 1) {
        ppuRegisters.writePpustatusVblank(1);
        // nmi request will be send after step is complete
        if(ppuRegisters.readPpuctrlVblankNMI()) eventQueue.get().push(EventType::InterruptNMI);
    }
}

void PPU::pixelRender() {
    switch (cycle) {
    case 0: return;
    case 1 ... 64: _spriteEvaluateClearSecondaryOAM();
    case 65 ... 256: _spriteEvaluate();
    case 257 ... 320: _spriteEvaluateFetchData();
    default: return;
    }

    if()
}

Address PPU::_getTileAddress() {
    // 0x2000 + nametable select + coarse scrolls
    return 0x2000 | (v & 0x0FFF);
}

Address PPU::_getAttributeAddress() {
    return 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07);
}

Address PPU::_getPatternLower(u8 index) {
    // if ppuctrl bit 4 is set, then we select the pattern table with base address 0x1000, else - with base address 0x0000.
    // internal pattern table address consists of 12 bits. Bit 3 is plane selector. Also we should add fine y scroll.
    return ((ppuRegisters.readPpuctrlBckgPTAddr() * 0x1000) | (index * 16)) + ((v & 0b111000000000000) >> 12);
    // higher = lower + 8
}

Address PPU::_getPatternLowerOAM(u8 index) {
    // 8x8 sprites
    if (ppuRegisters.readPpuctrlSpriteSize() == 0) return _getPatternLower(index);
    // 8x16 sprites
    else return ((index & 1) * 0x1000) | (((index & 0b11111110) >> 1) * 16) + ((v & 0b111000000000000) >> 12);
}

// coarse x is incremented when next tile is reached
void PPU::_coarseXIncrement() {
    if ((v & 0x001F) == 31) {
        v &= ~0x001F;
        v ^= 0x0400;    // switch horizontal nametable
    } else {
        v += 1;
    }
}

void PPU::_yIncrement() {
    // if fine Y < 7 increment fine y
    if ((v & 0x7000) != 0x7000) v += 0x1000;
    else {
        v &= ~0x7000;
        int y = (v & 0x03E0) >> 5;  // y = coarse Y
        if (y == 29) {
            y = 0;          // coarse Y = 0
            v ^= 0x0800;    // switch vertical nametable
        }
        else if (y == 31) y = 0;    // coarse Y = 0, nametable not switched
        else y += 1;                // increment coarse Y
        v = (v & ~0x03E0) | (y << 5);   // put coarse Y back into v
    }
}

// makes v x scroll = t x scroll (first 5 bits) (we can't restore fine x. Should we? Because It is cyclic?)
void PPU::_restoreXScrollFromT() {
    v ^= (v & 0b11111);
    v |= (t & 0b11111);
}

void PPU::_restoreYScrollFromT() {
    v ^= (v & 0b111001111100000);
    v |= (t & 0b111001111100000);
}

/*
    Should be called in some render cycles. Fetches different bytes depending on cycle.
*/
void PPU::_renderInternalFetchByte() {
    u8 remainder = (cycle - 1) % 8;
    switch(remainder) {
     // as byte fetching from memory requires 2 ppu cycles, we will get result on next cycle
    case 0: case 2: case 4: case 6: return;
    case 1: ntByte =  memory.read(_getTileAddress()); break;
    case 3: attrByte = memory.read(_getAttributeAddress()); break;
    case 5: lowBgByte = memory.read(_getPatternLower(ntByte)); break;
    case 7:
        highBgByte = memory.read(lowBgByte + 8);
        if(cycle != 256) _coarseXIncrement();
        // y is incremented only at dot 256 of each scanline
        else _yIncrement();
        break;
    }
}

// adding pattern table byte and attribute table byte to appropriate shift registers
void PPU::_renderInternalFedToShiftRegisters() {
    // storing pattern table bytes in upper 8 bits of each shift register(higher byte in reg[0])
    patternDataShifts16[0] ^= (patternDataShifts16[0] & 0xFF00);
    patternDataShifts16[0] |= (highBgByte << 8);
    patternDataShifts16[1] ^= (patternDataShifts16[1] & 0xFF00);
    patternDataShifts16[1] |= (lowBgByte << 8);
    // storing attribute table data in shifts(8 sequential pixel will have the same attribute table data)
    attrDataShifts8[0] = attrDataLatches[0] == 1 ? 0xFF : 0x00;
    attrDataShifts8[1] = attrDataLatches[1] == 1 ? 0xFF : 0x00;
    attrDataLatches[0] = attrByte & 1;
    attrDataLatches[1] = attrByte & 2;
}

void PPU::_renderInternalUnknownNTFetches() {
    if (cycle == 338 || cycle == 340) {
        // read and discard
        memory.read(_getTileAddress());
    }
}

void PPU::_spriteEvaluateClearSecondaryOAM() {
    // initting byte with 0xff on even cycles
    if (cycle % 2 == 0) {
        secondaryOAM[cycle / 2] = 0xff;
    }
}

// filling secondary OAM for NEXT scanline
void PPU::_spriteEvaluate() {
    // OAM[n*4 + m] - access to OAM sprite
    static u8 m = 0;
    static u8 n = 0;
    // current slot in secondary OAM
    static u8 secondarySlot = 0;
    // temporary value read on odd cycles
    static i16 tempVal;
    // initializing on cycle 65
    if (cycle == 65) {
        m = 0;
        n = 0;
        secondarySlot = 0;
    }
    // odd cycles - reading from OAM
    if (cycle % 2) {
        tempVal = OAM[n * 4 + m];
    }
    // even cycles - writing to secondary OAM
    else {
        secondaryOAM[secondarySlot * 4 + m] = tempVal;
        u8 tempY = secondaryOAM[secondarySlot * 4];
        // sprite is on current scanline
        if((tempY >= (scanline + 1) * 8) && (tempY < (scanline + 2) * 8)) {
            // all slots are already filled - setting sprite overflow
            if (secondarySlot == 8) ppuRegisters.writePpustatusSpriteOverflow(1);
            else {
                ++m;
                if(m == 4) {
                    m = 0;
                    ++n;
                    ++secondarySlot;
                }
            }
        } else {
            ++n;
        }
    }
}

// filling sprite shift registers, latches and counters
void PPU::_spriteEvaluateFetchData() {
    // to track 8x16 sprite's second parts
    static bool read8x16Occured = false;

    u8 remainder = (cycle - 1) % 8;
    u8 spriteIndex = (cycle - 257) / 8;

    switch(remainder) {
     // as byte fetching from memory requires 2 ppu cycles, we will get result on next cycle
    case 0: case 2: case 4: case 6: return;
    // garbage reads
    case 1: memory.read(_getTileAddress()); break;
    case 3: memory.read(_getAttributeAddress()); break;
    case 5:
        Address spriteLowPatternAddress = _getPatternLowerOAM(secondaryOAM[spriteIndex * 4 + 1]);
        // 8x8 sprites
        if(ppuRegisters.readPpuctrlSpriteSize() == 0) {
            if(read8x16Occured) if(logger) logger->log(LogLevel::Warning, "8x16 sprites rendering: second part was not read");
        }
        // 8x16 sprites
        else {
            if(!read8x16Occured) read8x16Occured = true;                    // read first part of sprite
            else { spriteLowPatternAddress++; read8x16Occured = false; }     // read second part of sprite(it's pattern data is stored at addr+1 of PT)
        }
        spriteLowPatternByte = spriteLowPatternAddress;
        break;
    case 7:
        spriteHighPatternByte = memory.read(spriteLowPatternByte + 8);
        break;
    }
}
