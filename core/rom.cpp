#include "rom.hpp"
#include <fstream>
#include <cstring>

NESHeaderFacade::NESHeaderFacade(NESHeader _nesHeader) : nesHeader{_nesHeader} {}

ROM::ROM(const std::string &fname, Logger* _logger)
    : logger{_logger}
{
    // open file
    std::ifstream romIfs;
    romIfs.open(fname, std::ios_base::binary);
    if(!romIfs) {
        if (logger) logger->log(LogLevel::Error, "Can't open file " + fname);
        throw InvalidROMException{};
    }

    NESHeader header;
    // read header
    try {
        header = _makeHeader(romIfs);
    } catch (InvalidROMException) {
        if (logger) logger->log(LogLevel::Error, "File " + fname + " is not valid NES file!");
        throw InvalidROMException{};
    }

    if(!_checkHeader(header)) {
        if (logger) logger->log(LogLevel::Error, "File " + fname + " is not valid NES file!");
        throw InvalidROMException{};
    }

    _header = Sptr<AbstractNESHeaderFacade>(new NESHeaderFacade(header));
    // !!! ADD TRAINER CHECK LATER

    _readOther(romIfs);
}

NESHeader ROM::_makeHeader(std::ifstream &romIfs) {
    Bytes<16> headerRaw;
    NESHeader header;
    romIfs.read(reinterpret_cast<char*>(&(headerRaw[0])), 16);
    if (headerRaw[0] != 'N' || headerRaw[1] != 'E' || headerRaw[2] != 'S' || headerRaw[3] != 0x1A) {
        throw InvalidROMException{};
    }
    memcpy(reinterpret_cast<void*>(&header), reinterpret_cast<void*>(&(headerRaw[4])), 6);
    return header;
}

// returns true if header is ok, else returns false
bool ROM::_checkHeader(const NESHeader& header) {
    bool warnings = false;
    if(header.persistentMemoryOnCartridge) { logger->log(LogLevel::Warning, "Persistent memory on cartridge is not supported!"); warnings = true; }
    if(header.trainer) { logger->log(LogLevel::Warning, "Trainer is not supported!"); warnings = true; }
    if(header.ignoreMirroringProvide4ScreenVRAM) { logger->log(LogLevel::Warning, "Ignore mirroring provide 4 screen VRAM is not supported!"); warnings = true; }
    if(header.vsUnisystem) { logger->log(LogLevel::Warning, "VS Unisystem is not supported!"); warnings = true; }
    if(header.playChoice) { logger->log(LogLevel::Warning, "PlayChoice is not supported!"); warnings = true; }
    if(header.nes2_0) { logger->log(LogLevel::Warning, "NES2.0 is not supported!"); warnings = true; }
    if(header.PRGRamSize8Kb) { logger->log(LogLevel::Warning, "PRGRAM is not supported!"); warnings = true; }
    if(header.tvSystem) { logger->log(LogLevel::Warning, "TV system flag is not supported!"); warnings = true; }
    return !warnings;
}

void ROM::_readOther(std::ifstream& romIfs) {
    u32 prgRomSize = header()->PRGROMSize16Kb() * 16 * 1024;
    u32 chrRomSize = header()->CHRROMSize8Kb() * 8 * 1024;
    _PRGROM.resize(prgRomSize);
    _CHRROM.resize(chrRomSize);

    romIfs.read(reinterpret_cast<char*>(&(_PRGROM[0])), prgRomSize);
    if(romIfs.gcount() != prgRomSize) {
        if(logger) logger->log(LogLevel::Error,  "File is not valid NES file: PRGROM corrupted!");
        throw InvalidROMException{};
    }
    romIfs.read(reinterpret_cast<char*>(&(_CHRROM[0])), chrRomSize);
    if(romIfs.gcount() != chrRomSize) {
        if(logger) logger->log(LogLevel::Error,  "File is not valid NES file: CHRROM corrupted!");
        throw InvalidROMException{};
    }
}
