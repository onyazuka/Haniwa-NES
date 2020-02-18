#pragma once
#include "core/cpu.hpp"
#include "core/ppu.hpp"
#include "core/rom.hpp"

class InvalidFileException{};

class NES {
public:
    NES(const std::string& romFname, Logger* logger=nullptr);

    inline void doInstruction() { cpu.execInstruction(); }
    void save(const std::string& fname);
    void load(const std::string& fname);

    inline ROM& getRom() { return rom; }
    inline PPU& getPpu() { return ppu; }
    inline CPU& getCpu() { return cpu; }
private:
    void waitUntilEventQueueIsEmpty();

    ROM rom;
    Uptr<MapperInterface> mapper;
    PPUMemory ppuMemory;
    EventQueue eventQueue;
    PPU ppu;
    Memory memory;
    CPU cpu;
    Logger* logger;
};
