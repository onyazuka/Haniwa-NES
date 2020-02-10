#pragma once
#include <array>
#include <optional>
#include <tuple>
#include <vector>
#include <queue>
#include <chrono>
#include <functional>
#include "common.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "eventqueue.hpp"
#include "log/log.hpp"

class UnknownOpcodeException {};
class UnknownAddressModeException {};
class UnknownCPUEventException {};

const Address ResetVectorAddress = 0xFFFC;
const Address InterruptVectorAddress = 0xFFFE;
const Address NonMaskableInterruptVectorAddress = 0xFFFA;
const std::chrono::duration CPUCycle = std::chrono::nanoseconds(558);   // roughly

struct Registers {
    enum class Flag { Carry, Zero, InterruptDisable, Decimal, Overflow, Negative };
    typedef std::vector<Flag> Flags;
    Registers();
    u8 A;       // arithmetic
    u8 X;       // multi-purpose
    u8 Y;       // multi-purpose
    u16 PC;     // program counter
    u8 S;       // stack
    u8 P;       // status

    inline bool carry() const { return P & 1; }
    inline bool zero() const { return P & 2; }
    inline bool interruptDisable() const { return P & 4; }
    inline bool decimal() const { return P & 8; }
    inline bool bFlag() const { return P & 16; }
    inline bool overflow() const { return P & 64; }
    inline bool negative() const { return P & 128; }

    inline Registers& setCarry(bool val) { P = val ? P | 0b00000001 : P & 0b11111110;  return *this; }
    template<typename T>
    inline Registers& setZero(T val) { P = val == 0 ? P | 0b00000010 : P & 0b11111101;  return *this; }
    inline Registers& setZero(bool val) { P = val ? P | 0b00000010 : P & 0b11111101;  return *this; }
    inline Registers& setInterruptDisable(bool val) { P = val ? P | 0b00000100 : P & 0b11111011;  return *this; }
    inline Registers& setDecimal(bool val) { P = val ? P | 0b00001000 : P & 0b11110111;  return *this; }
    // b flag is used by some instructions when pushing/pulling flag register to/from stack
    inline Registers& setBFlag(bool val) { P = val ? P | 0b00100000 : P & 0b11011111; return *this; }
    inline Registers& setOverflow(bool val) { P = val ? P | 0b01000000 : P & 0b10111111;  return *this; }
    template<typename T>
    inline Registers& setNegative(T val) { P = val & 0b10000000 ? P | 0b10000000 : P & 0b01111111;  return *this; }
    inline Registers& setNegative(bool val) { P = val ? P | 0b10000000 : P & 0b01111111;  return *this; }
};


struct Instruction {
    u8 val8;
    u16 val16;
    Address address;
    u8 length;          // in bytes
    u8 cycles;
};

class CPU {
public:
    CPU(Memory& _memory, PPU& _ppu, EventQueue& _eventQueue, Logger* _logger=nullptr);
    inline Memory& getMemory() { return memory; }
    inline Registers& registers() { return _registers; }
    void run();
    u8 step();

private:
    void interrupt(InterruptType, Address nextPC);
    void emulateCycles(std::function<int(void)> f);
    void oamDmaWrite();
    // stack operations
    CPU& push(u8 val) { memory.write8((registers().S)--, val); return *this; }
    inline CPU& push(u16 val) { memory.write16((registers().S), val); registers().S -= 2; return *this; }
    inline CPU& pop8() { (registers().S)++; return *this; }
    inline CPU& pop16() { (registers().S) += 2; return *this; }
    u8 top8() { return memory.read8(registers().S); }
    u16 top16() { return memory.read16(registers().S); }
    AddressationMode _getAddressationModeByOpcode(u8 opcode);
    void _processEventQueue();

    const Address ROMOffset = 0xC000;
    Registers _registers;
    Memory& memory;
    PPU& ppu;
    // different parts of NES can initialize different kinds of events: interrputs, OAMDMA write etc. Those events are added in the eventQueue
    // and processed after competion of current CPU instruction in FIFO order.
    EventQueue eventQueue;
    Logger* logger;
};

Instruction makeInstruction(CPU& cpu, AddressationMode addrMode, Address offset);

std::string getPrettyInstruction(u8 opcode, AddressationMode addrMode, Address curAddress, Instruction instruction);
