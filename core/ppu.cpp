#include <cstring>
#include <iostream>
#include "include/ppu.hpp"

PPURegisters::PPURegisters()
    : ppuctrl{0}, ppumask{0}, ppustatus{0}, oamaddr{0}, ppuscroll{0}, ppuaddr{0}, ppudata{0} {}

PPURegistersAccess::PPURegistersAccess(PPU &_ppu)
    : ppu{_ppu} {}

PPURegistersAccess& PPURegistersAccess::writePpuctrl(u8 val) {
    // writing base nametable address to ppu's register t
    ppuRegisters.ppuctrl = val;
    ppu.t ^= (ppu.t & 0b110000000000);
    ppu.t |= (val & 0b11) << 10;
    // if setting NMI flag, and in vblank, generate NMI
    if ((ppuRegisters.ppustatus & 0b10000000) && (val & 0b10000000) && !(ppuRegisters.ppuctrl & 0b10000000)) {
        ppu.eventQueue.get().push(EventType::InterruptNMI);
    }
    return *this;
}

PPURegistersAccess& PPURegistersAccess::writePpuctrlNametableBase(u8 val) {
    (val & 1) ? setBit(ppuRegisters.ppuctrl, 0) : clearBit(ppuRegisters.ppuctrl, 0);
    (val & 2) ? setBit(ppuRegisters.ppuctrl, 1) : clearBit(ppuRegisters.ppuctrl, 1);
    ppu.t |= (val) << 10;
    return *this;
}

u8 PPURegistersAccess::readPpustatus() {
    u8 res = ppuRegisters.ppustatus;
    ppu.w = 0;
    clearBit(ppuRegisters.ppustatus, 7);
    return res;
}

PPURegistersAccess& PPURegistersAccess::writeOamdata(u8 val) {
    if (ppu.isDataChangeForbidden()) return *this;
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
        ppu.t |= ((val & 0b11111000) >> 3);
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
    ppuRegisters.ppuaddr = ppu.v & ppu.AccessAddressMask;
    return *this;
}

PPURegistersAccess& PPURegistersAccess::writePpudata(u8 val) {
    // IS THIS NEEDED? Monkey is not drawn in Donkey Kong with it
    //if (ppu.isDataChangeForbidden()) return *this;
    ppu.memory.write(ppu.v & ppu.AccessAddressMask, val);
    // if ppuctrl 2nd bit is set, incrementing by 32, else by 1
    ppu.v += readPpuctrlVramIncrement() ? 32 : 1;
    return *this;
}

u8 PPURegistersAccess::readPpudata() const {
    // internal buffer
    static u8 buf;
    u8 readData = ppu.memory.read(ppu.v & ppu.AccessAddressMask);
    // when reading from before palettes, return data from internal buffer, but update it
    if(ppu.v < 0x3F00) {
        u8 res = buf;
        buf = readData;
        ppu.v += readPpuctrlVramIncrement() ? 32 : 1;
        return res;
    }
    // reading here works differently - updating buffer AND returning currently read data
    else {
        // palette
        buf = readData;
        ppu.v += readPpuctrlVramIncrement() ? 32 : 1;
        return readData;
    }
}

PPURegistersAccess& PPURegistersAccess::writeOamdma(u8 val) {
    ppuRegisters.oamdma = val;
    ppu.eventQueue.get().push(EventType::OAMDMAWrite);
    return *this;
}

PPU::PPU(PPUMemory& _memory, EventQueue& _eventQueue, Logger* _logger)
    : Observable(), ppuRegisters{*this}, memory{_memory}, eventQueue{_eventQueue}, logger{_logger}, v{0}, t{0}, x{0}, w{0}, attrDataLatches{}, OAM{},
      secondaryOAM{}, spritesPatternDataShifts8{}, spriteAttributeBytes{}, spriteXCounters{}, frame{0}, scanline{-1}, cycle{0}, drawDebugGrid{false}, _image{} {}

void PPU::step() {
    switch(scanline) {
        case -1: preRender(); pixelRender(); break;
        case 0 ... 239: visibleRender(); pixelRender(); break;
        case 240: postRender(); break;
        case 241 ... 260: verticalBlank(); break;
    }
    // each scanline consists of exactly 341 cycles
    ++cycle;
    if (cycle == 341) {
        ++scanline;
        cycle = 0;
    }
    // odd frames skip 262 prerender, even - don't skip
    else if ((scanline == -1) && (frame % 2) && (cycle == 340)) {
        cycle = 0;
        ++scanline;
    }
    if (scanline == 260) {
        // to prerender
        scanline = -1;
        cycle = 0;
        ++frame;
    }
}

// should last roughly PPUCycle nanoseconds
void PPU::emulateCycle() {
    step();
}

/*
    Checks if secondaryOAM[secondaryOAMIndex] is sprite zero(OAM[0]-OAM[3])
*/
bool PPU::isItSprite0(u8 secondaryOAMIndex) const {
    return (secondaryOAM[secondaryOAMIndex * 4] == OAM[0] && secondaryOAM[secondaryOAMIndex * 4 + 1] == OAM[1] &&
            secondaryOAM[secondaryOAMIndex * 4 + 2] == OAM[2] && secondaryOAM[secondaryOAMIndex * 4 + 3] == OAM[3]);
}

Serialization::BytesCount PPU::serialize(std::string &buf) {
    auto& regs = ppuRegisters.ppuRegisters;
    return Serialization::Serializer::serializeAll(buf, &regs.ppuctrl, &regs.ppumask, &regs.ppustatus, &regs.oamaddr, &regs.oamdata,
                                                   &regs.ppuscroll, &regs.ppuaddr, &regs.ppudata, &regs.oamdma, &memory.getMemory(),
                                                   &v, &t, &x, &w, &patternDataShifts16, &attrDataShifts8, &attrDataLatches,
                                                   &ntByte, &attrByte, &lowBgByte, &highBgByte, &OAM, &secondaryOAM, &ppuMap.bckgMap, &ppuMap.spriteMap,
                                                   &spritesPatternDataShifts8, &spriteAttributeBytes, &spriteXCounters, &spriteLowPatternByte,
                                                   &spriteHighPatternByte, &frame, &scanline, &cycle);
}

Serialization::BytesCount PPU::deserialize(const std::string &buf, Serialization::BytesCount offset) {
    auto& regs = ppuRegisters.ppuRegisters;
    using namespace Serialization;
    auto memWr  = wrapArr(memory.getMemory());
    auto pd16Wr = wrapArr(patternDataShifts16);
    auto ad8Wr  = wrapArr(attrDataShifts8);
    auto adlWr  = wrapArr(attrDataLatches);
    auto oamWr  = wrapArr(OAM);
    auto soamWr = wrapArr(secondaryOAM);
    auto mapBWr = wrapArr(ppuMap.bckgMap);
    auto mapSWr = wrapArr(ppuMap.spriteMap);
    auto spd8Wr = wrapArr(spritesPatternDataShifts8);
    auto sadWr  = wrapArr(spriteAttributeBytes);
    auto scWr   = wrapArr(spriteXCounters);
    return Serialization::Deserializer::deserializeAll(buf, offset, &regs.ppuctrl, &regs.ppumask, &regs.ppustatus, &regs.oamaddr, &regs.oamdata,
                                                   &regs.ppuscroll, &regs.ppuaddr, &regs.ppudata, &regs.oamdma, &memWr,
                                                   &v, &t, &x, &w, &pd16Wr, &ad8Wr, &adlWr,
                                                   &ntByte, &attrByte, &lowBgByte, &highBgByte, &oamWr, &soamWr, &mapBWr, &mapSWr, &spd8Wr,
                                                   &sadWr, &scWr, &spriteLowPatternByte, &spriteHighPatternByte,
                                                   &frame, &scanline, &cycle);
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
    if(cycle > 329 && cycle <= 337) {
        _renderInternalBckgShifts();
    }
    // updating shifters
    if(cycle == 329 || cycle == 337) {
        _renderInternalFedRegisters();
    }
    // unknown NT fetches
}

void PPU::visibleRender() {
    switch(cycle) {
    case 0: return;
    case 1 ... 256:
        _renderInternalFetchByte();
        break;
    case 257:
        _restoreXScrollFromT();
        // DON'T NEED BREAK HERE
    case 258 ... 320:
        // all work is done in pixel evaluate for those cycles
        break;
    case 321 ... 336:
        _renderInternalFetchByte();
        break;
    case 337 ... 340:
        // unknown NT fetches
        break;
    }
    if(cycle >= 3 && cycle <= 258) {
        drawBackgroundPixel(cycle - 3, scanline);
        _renderInternalBckgShifts();
    }
    if(cycle > 329 && cycle <= 337) _renderInternalBckgShifts();
    if(((cycle > 1) && (cycle <= 257) && (((cycle - 1) % 8) == 0)) || (cycle == 329 || cycle == 337) ) {
        _renderInternalFedRegisters();
    }
}

// nice stage, just idling here
void PPU::postRender() {
    return;
}

// just turning on vblank on cycle number 1(SECOND cycle)
void PPU::verticalBlank() {
    if (scanline == 241 && cycle == 1) {
        ppuRegisters.writePpustatusVblank(1);
        notify((int)PPUEvent::RerenderMe);
        // nmi request will be send after step is complete
        if(ppuRegisters.readPpuctrlVblankNMI()) eventQueue.get().push(EventType::InterruptNMI);
    }
}

void PPU::pixelRender() {
    switch (cycle) {
    case 0: return;
    case 1 ... 64: _spriteEvaluateClearSecondaryOAM(); break;
    case 65 ... 256: _spriteEvaluate(); break;
    // OAMADDR is set to 0 during those ticks
    case 257 ... 320: _spriteEvaluateFetchData(); ppuRegisters.writeOamaddr(0); break;
    }
    // it should be called AFTER 258 cycle background pixel rendering. There is exactly 8 cycles to draw sprite line before it's registers will be cleared.
    if(cycle >= 258 && cycle <= 321) drawSpritePixel(scanline);
    if((cycle >= 265 && cycle <= 321) && (((cycle - 1) % 8) == 0)) _spriteEvaluateFedData();
}

void PPU::drawBackgroundPixel(u8 xCoord, u8 yCoord) {
    static const auto& ppuMem = memory.getMemory();
    // get background pixel
    // keeping in mind fine x scroll
    u16 shiftMask16 = 0b1000000000000000 >> x;
    u8  shiftMask8  = 0b10000000 >> x;
    // OPTIMIZATION: memory access operation with all its check may by expensive, so, as we access only palette here, we can read memory directly
    u32 bckgColor = Palette[ppuMem[0x3f00]];
    bool bckgTransparent = true;
    // if show background/leftmost 8 background
    if((ppuRegisters.ppuRegisters.ppumask & 0b1000) && !(xCoord < 8 && !(ppuRegisters.ppuRegisters.ppumask & 0b10))) {
        u8 bckgPaletteInnerIndex = (patternDataShifts16[0] & shiftMask16 ? 2 : 0) + ((patternDataShifts16[1] & shiftMask16) ? 1 : 0);
        u8 bckgPaletteNumber = (attrDataShifts8[0] & shiftMask8 ? 2 : 0) + (attrDataShifts8[1] & shiftMask8 ? 1 : 0);
        // if bckgPaletteInnerIndex is 0, we should use the backdrop color
        if(bckgPaletteInnerIndex != 0) {
            Address bckgPaletteAddress = 0x3F00 + (bckgPaletteNumber << 2) + bckgPaletteInnerIndex;
            bckgColor = Palette[ppuMem[bckgPaletteAddress]];
            bckgTransparent = !(bckgPaletteAddress & 0b11);
        }
        _image[(yCoord << 8) + xCoord] = bckgColor;
    }
    else {
        _image[(yCoord << 8) + xCoord] = getForcedBlankColor();
    }
    // clearing map here - true means transparent
    ppuMap.setSprite(xCoord, true);
    ppuMap.setBckg(xCoord, bckgTransparent);

    // drawing grid for debug
#ifdef DEBUG
    if(drawDebugGrid && (xCoord % 8 == 0 || yCoord % 8 == 0)) {
        _image[yCoord * 256 + xCoord] = 0xffffff;
        return;
    }
#endif
}

// should be called AFTER all background pixels are drawn
void PPU::drawSpritePixel(u8 yCoord) {
    static const auto& ppuMem = memory.getMemory();
    u8 spriteToDraw = (cycle - 258) >> 3;
    u8 i = spriteToDraw;
    u8 spriteXOffset = (cycle - 258) & 7;
    u16 spriteXCoord = spriteXCounters[spriteToDraw] + spriteXOffset;

    if(spriteXCoord >= 256) return;
    // if not show sprites - return
    if (!(ppuRegisters.ppuRegisters.ppumask & 0b10000 || (spriteXCoord < 8 && ppuRegisters.ppuRegisters.ppumask & 0b100))) {
        _image[yCoord * 256 + spriteXCoord] = getForcedBlankColor();
        return;
    }

    bool bckgTransparent = ppuMap.testBckg(spriteXCoord);
    u8 spritePaletteInnerIndex = (spritesPatternDataShifts8[i * 2] & 0b10000000 ? 2 : 0) + (spritesPatternDataShifts8[i * 2 + 1] & 0b10000000 ? 1 : 0);
    u8 spritePaletteNumber = spriteAttributeBytes[i] & 0b11;
    Address spritePaletteAddress;
    bool spriteTransparent;
    if(spritePaletteInnerIndex != 0) {
        spritePaletteAddress = 0x3F10 + (spritePaletteNumber << 2) + spritePaletteInnerIndex;
        spriteTransparent = !(spritePaletteAddress & 0b11);
    }
    // using backdrop
    else {
        spritePaletteAddress = 0x3F00;
        spriteTransparent = true;
    }
    u32 spriteColor = Palette[ppuMem[spritePaletteAddress]];
    u8 spritePriority = (spriteAttributeBytes[i] & 0b00100000) >> 5;

    // if not have any or have a transparent sprite here - override
    if(ppuMap.testSprite(spriteXCoord)) {
        _image[yCoord * 256 + spriteXCoord] = colorMultiplexer(bckgTransparent, _image[(yCoord << 8) + spriteXCoord], spriteTransparent, spriteColor, spritePriority);
    }

    ppuMap.setSprite(spriteXCoord, spriteTransparent);
    spritesPatternDataShifts8[i * 2] <<= 1;
    spritesPatternDataShifts8[i * 2 + 1] <<= 1;

    // sprite 0 hit
    if (!(ppuRegisters.ppuRegisters.ppustatus & 0b01000000) && spriteXCoord != 255 && !bckgTransparent && !spriteTransparent && isItSprite0(i)) {
        ppuRegisters.writePpustatusSprite0Hit(1);
    }
}

u32 PPU::colorMultiplexer(bool bckgTransparent, u32 bckgColor, bool spriteTransparent, u32 spriteColor, u8 spritePriority) {
    if(bckgTransparent && spriteTransparent) return bckgColor;
    if(bckgTransparent && !spriteTransparent) return spriteColor;
    if(!bckgTransparent && spriteTransparent) return bckgColor;
    // !bckgTransparent && !spriteTransparent
    if(spritePriority == 0) return spriteColor; // 0 means in front of background
    else return bckgColor;
}

/*
    During forced blank(when rendering is disabled) we should render sprites of BACKDROP color.
    Usually, it is the color at 0x3F00, BUT if v points to an address in the palette(0x3F00 - 0x3FFF) we should use this value.
    This is so called "background palette hack".
*/
u32 PPU::getForcedBlankColor() const {
    Address addr = v & AccessAddressMask;
    if (addr >= 0x3F00 && addr <= 0x3FFF) return Palette[memory.read(addr)];
    else return Palette[0x3F00];
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
    u8 spriteIndex = (cycle - 257) / 8;
    bool flipVertical = secondaryOAM[spriteIndex * 4 + 2] & 0b10000000;
    // vertical flip can be described as reverse of current fine y scroll
    u8 fineYScroll = scanline + 1 - secondaryOAM[spriteIndex * 4];
    if (flipVertical) fineYScroll = ~fineYScroll;
    // 8x8 sprites
    if (ppuRegisters.readPpuctrlSpriteSize() == 0) {
        // 8x16 sprites aligned in pattern table as: lower1, higher1, lower2, higher2
        u8 ptOffset = fineYScroll < 8 ? fineYScroll : (fineYScroll + 8);
        return ((ppuRegisters.readPpuctrlSpritePTAddrt() * 0x1000) | (index * 16)) + ptOffset;
    }
    // 8x16 sprites
    else {
        return (((index & 1) * 0x1000) | (((index & 0b11111110) >> 1) * 16)) + fineYScroll;
    }
}

// coarse x is incremented when next tile is reached
void PPU::_coarseXIncrement() {
    // not incrementing if render is disabled
    if (renderingDisabled()) return;
    if ((v & 0x001F) == 31) {
        v &= ~0x001F;
        v ^= 0x0400;    // switch horizontal nametable
    } else {
        v += 1;
    }
}

void PPU::_yIncrement() {
    // not incrementing if render is disabled
    if (renderingDisabled()) return;
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
    if (renderingDisabled()) return;
    // bit 10 is a part of nametable selection, and also is part of x scroll
    v ^= (v & 0b10000011111);
    v |= (t & 0b10000011111);
}

void PPU::_restoreYScrollFromT() {
    if (renderingDisabled()) return;
    // don't forget nametable bit
    v ^= (v & 0b111101111100000);
    v |= (t & 0b111101111100000);
}

/*
    Should be called in some render cycles. Fetches different bytes depending on cycle.
*/
void PPU::_renderInternalFetchByte() {
    // why read twice if we can read once?
    static Address lowBgByteAddr = 0;
    u8 remainder = (cycle - 1) & 7;
    switch(remainder) {
     // as byte fetching from memory requires 2 ppu cycles, we will get result on next cycle
    case 0: case 2: case 4: case 6: return;
    case 1: ntByte =  memory.readDirectly(_getTileAddress()); break;
    case 3: {
        u8 tempAttrByte = memory.readDirectly(_getAttributeAddress());
        u8 tileY = (v & 0b1111100000) >> 5;
        u8 tileX = (v & 0b0000011111);

        // each 2 bits of attribute byte contain info about a 2x2 tile piece
        bool tileXOdd = (tileX >> 1) & 1;
        bool tileYOdd = (tileY >> 1) & 1;
        u8 attrByteOffset = (tileYOdd ? 4 : 0) + (tileXOdd ? 2 : 0);

        attrByte = 0;
        // attribute higher is stored in bit 1, lower - in bit 0
        attrByte += (tempAttrByte & (1 << (attrByteOffset + 1))) ? 2 : 0;
        attrByte += (tempAttrByte & (1 << attrByteOffset)) ? 1 : 0;
        break;
    }
    case 5:
        lowBgByteAddr = _getPatternLower(ntByte);
        lowBgByte = memory.readCHR(lowBgByteAddr);
        break;
    case 7:
        highBgByte = memory.readCHR(lowBgByteAddr + 8);
        if(cycle != 256) _coarseXIncrement();
        // y is incremented only at dot 256 of each scanline
        else _yIncrement();
        break;
    }
}

// adding pattern table byte and attribute table byte to appropriate shift registers
void PPU::_renderInternalFedRegisters() {
    // storing pattern table bytes in LOWER 8 bits of each shift register(higher byte in reg[0])
    patternDataShifts16[0] ^= (patternDataShifts16[0] & 0x00FF);
    patternDataShifts16[0] |= highBgByte;
    patternDataShifts16[1] ^= (patternDataShifts16[1] & 0x00FF);
    patternDataShifts16[1] |= lowBgByte;

    attrDataLatches[0] = attrByte & 0b10;
    attrDataLatches[1] = attrByte & 0b01;
}

void PPU::_renderInternalBckgShifts() {
    patternDataShifts16[0] <<= 1;
    patternDataShifts16[1] <<= 1;
    attrDataShifts8[0] <<= 1;
    attrDataShifts8[1] <<= 1;
    // setting lower attribute bit from appropriate latch
    attrDataShifts8[0] |= attrDataLatches[0];
    attrDataShifts8[1] |= attrDataLatches[1];
}

void PPU::_spriteEvaluateClearSecondaryOAM() {
    // initting byte with 0xff on even cycles
    if (cycle % 2 == 0) {
        secondaryOAM[(cycle - 1) / 2] = 0xff;
    }
}

// filling secondary OAM for NEXT scanline
void PPU::_spriteEvaluate() {
    // sprite evaluation not occures if rendering is disabled
    if(renderingDisabled()) return;
    // OAM[n*4 + m] - access to OAM sprite
    static u8 m = 0;
    static u16 n = 0;
    // current slot in secondary OAM
    static u8 secondarySlot = 0;
    // initializing on cycle 65. First sprite is the one at oamaddr
    if (cycle == 65) {
        m = ppuRegisters.readOamaddr() % 4;
        n = ppuRegisters.readOamaddr() / 4;
        secondarySlot = 0;
    }
    // even cycles - writing to secondary OAM
    if (!(cycle % 2)) {
        // if oamaddr at start of dot 65 is not 0, overflow can occure. In this case, just ignoring next values
        if (n >= 64) return;
        u8 tempY = OAM[n * 4];
        // sprite is on the next scanline
        u8 spriteHeight = ppuRegisters.readPpuctrlSpriteSize() ? 16 : 8;
        if((secondarySlot < 8) && tempY <= (scanline + 1) && (scanline + 1) < (tempY + spriteHeight)) {
            secondaryOAM[secondarySlot * 4 + m] = OAM[n * 4 + m];
            // all slots are already filled - setting sprite overflow
            if (secondarySlot == 8) ppuRegisters.writePpustatusSpriteOverflow(1);
            ++m;
            if(m == 4) { ++secondarySlot; m = 0; ++n; }
        }
        else { m = 0; ++n; }
    }
}

// fetching data and writing it to the temporary registers
void PPU::_spriteEvaluateFetchData() {
    static Address pbAddr = 0;
    u8 remainder = (cycle - 1) & 7;
    u8 spriteIndex = (cycle - 257) >> 3;
    switch(remainder) {
     // as byte fetching from memory requires 2 ppu cycles, we will get result on next cycle
    case 0: case 2: case 4: case 6: return;
    // garbage reads NOT NEEDED
    case 1: break;
    case 3: break;
    case 5: pbAddr = _getPatternLowerOAM(secondaryOAM[spriteIndex * 4 + 1]); spriteLowPatternByte = memory.read(pbAddr); break;
    case 7: spriteHighPatternByte = memory.read(pbAddr + 8); break;
    }
}

// filling sprite shift registers, latches and counters
void PPU::_spriteEvaluateFedData() {
    u8 spriteIndex = (cycle - 265) / 8;
    u8 flipHorizontal = secondaryOAM[spriteIndex * 4 + 2] & 0b01000000;
    spritesPatternDataShifts8[spriteIndex * 2] = flipHorizontal ? reverseByte(spriteHighPatternByte) : spriteHighPatternByte;
    spritesPatternDataShifts8[spriteIndex * 2 + 1] = flipHorizontal ? reverseByte(spriteLowPatternByte) : spriteLowPatternByte;
    spriteAttributeBytes[spriteIndex] = secondaryOAM[spriteIndex * 4 + 2];
    spriteXCounters[spriteIndex] = secondaryOAM[spriteIndex * 4 + 3];
}
