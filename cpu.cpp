#include "core/cpu.hpp";

namespace CPU {

    // P = 0x34 - interruptions disabled, b-bits are set
    Registers::Registers()
        : A{0}, X{0}, Y{0}, P{0x34}, S{0xFD} {}

    CPU::CPU(Memory &_memory)
        : memory{_memory} {}

    CPU::step() {
        u8 opcode = memory.read8(ROMOffset + PC);
        switch(opcode) {

        }
    }
}
