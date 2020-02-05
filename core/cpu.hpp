#pragma once
#include <array>
#include <optional>
#include "common.hpp"
#include "memory.hpp"

namespace CPU {

    struct Registers {
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
        inline bool overflow() const { return P & 64; }
        inline bool negative() const { return P & 128; }

        inline Registers& setCarry(bool val) { P = val ? P | 0b00000001 : P & 0b11111110;  return *this; }
        inline Registers& setZero(bool val) { P = val ? P | 0b00000010 : P & 0b11111101;  return *this; }
        inline Registers& setInterruptDisable(bool val) { P = val ? P | 0b00000100 : P & 0b11111011;  return *this; }
        inline Registers& setDecimal(bool val) { P = val ? P | 0b00001000 : P & 0b11110111;  return *this; }
        inline Registers& setOverflow(bool val) { P = val ? P | 0b01000000 : P & 0b10111111;  return *this; }
        inline Registers& setNegative(bool val) { P = val ? P | 0b10000000 : P & 0b01111111;  return *this; }
    };

    enum class InstructionName {
        ADC, AND, ASL, BCC, BCS, BEQ, BIT, BMI, BNE, BPL, BRK, BVC, BVS, CLC, CLD, CLI,
        CLV, CMP, CPX, CPY, DEC, DEX, DEY, EOR, INC, INX, INY, JMP, JSR, LDA, LDX, LDY,
        LSR, NOP, ORA, PHA, PHP, PLA, PLP, ROL, ROR, RTI, RTS, SBC, SEC, SED, SEI, STA,
        STX, STY, TAX, TAY, TSX, TXA, TXS, TYA
    };

    typedef std::pair<InstructionName, AdressationMode> InstructionDescr;



    template<typename OpType1, typename OpType2>
    struct Instruction {
        std::optional<OpType1> operand1;
        std::optional<OpType2> operand2;
    };

    class CPU {
    public:
        CPU(Memory& _memory);
        inline Registers& registers() { return _registers; }
        void step();
    private:
        const Offset ROMOffset = 0xC000;
        Registers _registers;
        Memory& memory;
    };

}
