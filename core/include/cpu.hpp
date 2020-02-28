#pragma once
#include <array>
#include <optional>
#include <tuple>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <functional>
#include "common.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "eventqueue.hpp"
#include "log/log.hpp"
#include "serialize/serializer.hpp"

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


class Instruction {
public:
    Instruction() {}
    Instruction(Memory* _memory, std::optional<u8> _val8, Address _address, u16 arg, u8 _length, u8 _cycles, AddressationMode addrMode, Address offset, u8 opcode)
        : address{_address}, argument{arg}, length{_length}, cycles{_cycles}, addrMode{addrMode}, offset{offset}, opcode{opcode}, memory{_memory}, value8{_val8} {}
    Address address;
    u16 argument;
    u8 length;          // in bytes
    u8 cycles;
    AddressationMode addrMode;
    Address offset;
    u8 opcode;
    // value will be read only once and only if needed
    inline u8 val8() { if(!value8) value8 = memory->read8(address); return value8.value(); }
    inline bool hasVal8() const { return value8.has_value(); }
private:
    Memory* memory;
    std::optional<u8> value8;
};

class CPU : public Serialization::Serializable, public Serialization::Deserializable {
public:
    CPU(Memory& _memory, PPU& _ppu, EventQueue& _eventQueue, Logger* _logger=nullptr);
    inline Memory& getMemory() { return memory; }
    inline Registers& registers() { return _registers; }
    inline bool eventQueueEmpty() const { return eventQueue.get().empty(); }
    inline auto getInstructionCounter() const { return instructionCounter; }
    void run();
    // with all synchonizations
    void exec();
    inline std::thread runInSeparateThread() { return std::thread([this] { run(); }); }

    // serialization
    Serialization::BytesCount serialize(std::string &buf);
    Serialization::BytesCount deserialize(const std::string &buf, Serialization::BytesCount offset);

private:
    Instruction fetchInstruction();
    u8 executeInstruction(Instruction& instruction);

    void interrupt(InterruptType, Address nextPC);
    void emulateCycles(std::function<int(void)> f, bool processEvents);
    void oamDmaWrite();
    // stack operations
    inline CPU& push(u8 val) { memory.write8((0x100 + registers().S), val); registers().S -= 1; return *this; }
    inline CPU& push(u16 val) { memory.write16(0x100 + registers().S - 1, val); registers().S -= 2; return *this; }
    inline CPU& pop8() { registers().S += 1; return *this; }
    inline CPU& pop16() { registers().S += 2; return *this; }
    u8 top8() { return memory.read8(0x100 + registers().S + 1); }
    u16 top16() { return memory.read16(0x100 + registers().S + 1); }
    void _frameSync();
    void _processEventQueue();

    const Address ROMOffset = 0xC000;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> syncTimePoint;
    Registers _registers;
    Memory& memory;
    PPU& ppu;
    // different parts of NES can initialize different kinds of events: interrputs, OAMDMA write etc. Those events are added in the eventQueue
    // and processed after competion of current CPU instruction in FIFO order.
    EventQueue& eventQueue;
    Logger* logger;
    // used for debugging
    u64 instructionCounter;
};

Instruction makeInstruction(CPU& cpu, AddressationMode addrMode, Address offset, u8 opcode);

std::string getPrettyInstruction(u8 opcode, AddressationMode addrMode, Address curAddress, Instruction instruction);
