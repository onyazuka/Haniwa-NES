#pragma once
#include <string>
#include "common.hpp"
#include "log/log.hpp"

// total size = 6 bytes
struct NESHeader {
    u8 PRGROMSize16Kb;
    u8 CHRROMSize8Kb;   // 0 means that we use CHR RAM
    u8 mirroring : 1;
    u8 persistentMemoryOnCartridge : 1;
    u8 trainer : 1;
    u8 ignoreMirroringProvide4ScreenVRAM : 1;
    u8 mapperLower : 4;
    u8 vsUnisystem : 1;
    u8 playChoice : 1;
    u8 nes2_0 : 2;
    u8 mapperUpper : 4;
    // NES2.0 is not supported yet, so flags 8-15 here are in simple NES format
    u8 PRGRamSize8Kb;
    u8 tvSystem;
    // FLAG 10 is not implemented yet
};

// makes standard access to INES or NES2.0 header
class AbstractNESHeaderFacade {
public:
    virtual u8 PRGROMSize16Kb() const = 0;
    virtual u8 CHRROMSize8Kb() const = 0;
    virtual Mirroring mirroring() const = 0;
    virtual u8 mapper() const = 0;
};

class NESHeaderFacade : public AbstractNESHeaderFacade {
public:
    NESHeaderFacade(NESHeader _nesHeader);
    inline u8 PRGROMSize16Kb() const { return nesHeader.PRGROMSize16Kb; }
    inline u8 CHRROMSize8Kb() const { return nesHeader.CHRROMSize8Kb; }
    inline Mirroring mirroring() const { return Mirroring(nesHeader.mirroring); }
    inline u8 mapper() const { return (nesHeader.mapperUpper << 4) + nesHeader.mapperLower; }
private:
    NESHeader nesHeader;
};

class InvalidROMException {};

class ROM {
public:
    ROM(const std::string& fname, Logger* _logger=nullptr);
    inline const Sptr<AbstractNESHeaderFacade> header() const { return _header; }
    inline DinBytes& PRGROM() { return _PRGROM; }
    inline DinBytes& CHRROM() { return _CHRROM; }
private:
    NESHeader _makeHeader(std::ifstream& romIfs);
    bool _checkHeader(const NESHeader& header);
    void _checkMapper();
    void _readOther(std::ifstream& romIfs);

    Sptr<AbstractNESHeaderFacade> _header;
    DinBytes _PRGROM;
    DinBytes _CHRROM;
    Logger* logger;
};
