#pragma once
#include "core/include/cpu.hpp"
#include "core/include/ppu.hpp"
#include "core/include/rom.hpp"
#include "core/include/input.hpp"

class InvalidFileException{};

class NES {
public:
    NES(const std::string& romFname, Logger* logger=nullptr);

    inline void doInstruction() { cpu.exec(); }
    void save(const std::string& fname);
    void load(const std::string& fname);

    inline ROM& getRom() { return rom; }
    inline PPU& getPpu() { return ppu; }
    inline CPU& getCpu() { return cpu; }
    inline StandardController& getController(int num) { if(num == 0) return stController1; else return stController2; }
private:
    void waitUntilEventQueueIsEmpty();

    ROM rom;
    StandardController stController1;
    StandardController stController2;
    Sptr<MapperInterface> mapper;
    PPUMemory ppuMemory;
    EventQueue eventQueue;
    PPU ppu;
    Memory memory;
    CPU cpu;
    Logger* logger;
};
