#include "include/cpu.hpp"
#include <string>
#include <thread>
#include <chrono>
#include <iostream>

// P = 0x34 - interruptions disabled, b-bits are set
Registers::Registers()
    : A{0}, X{0}, Y{0}, S{0xFD}, P{0x34} {}

/*
    offset should point to current opcode
    return address, 8bit value read from the address, 16 bit value, length of instruction and cycles count(basic minimal value for this addressation mode)
        (some special instruction like BRK may change this value later)
*/
Instruction makeInstruction(CPU& cpu, AddressationMode addrMode, Address offset, u8 opcode) {
    // skipping opcode
    ++offset;
    Address addr = 0;
    u16 argument = 0;
    u8 length = 1;      // at least opcode
    u8 cycles = 0;
    std::optional<u8> val8 = std::nullopt;
    Memory& memory = cpu.getMemory();
    const Registers& registers = cpu.registers();
    // cross page can occure when using AbsoluteX, AbsoluteY or IndirectIndexed modes
    bool crossPage = false;
    if (addrMode == AddressationMode::Implied || addrMode == AddressationMode::Accumulator) {
        addr = 0;
        argument = 0;
        length = 1;
    }
    else if (addrMode == AddressationMode::Immediate) {
        addr = 0;
        argument = 0;
        length = 2;
        val8 = memory.read8(offset);
    }
    else if (addrMode == AddressationMode::ZeroPage) {
        addr = memory.read8(offset);
        argument = addr;
        length = 2;
    }
    else if (addrMode == AddressationMode::ZeroPageX) {
        argument = memory.read8(offset);
        addr = (argument + registers.X) % 0x100;
        length = 2;
    }
    else if (addrMode == AddressationMode::ZeroPageY) {
        argument = memory.read8(offset);
        addr = (argument + registers.Y) % 0x100;
        length = 2;
    }
    else if (addrMode == AddressationMode::Relative) {
        argument = memory.read8(offset);
        // +1 because relative offset is calculated from place AFTER the instruction(and any relative addressing instruction has size 2 bytes)
        addr = offset + 1 + (i8)(argument);
        length = 2;
    }
    else if (addrMode == AddressationMode::Absolute) {
        argument = memory.read16(offset);
        addr = argument;
        length = 3;
    }
    else if (addrMode == AddressationMode::AbsoluteX) {
        argument = memory.read16(offset);
        addr = argument + registers.X;
        crossPage = (argument & 255) != (addr & 255);
        length = 3;
    }
    else if (addrMode == AddressationMode::AbsoluteY) {
        argument = memory.read16(offset);
        addr = argument + registers.Y;
        crossPage = (argument & 255) != (addr & 255);
        length = 3;
    }
    else if (addrMode == AddressationMode::Indirect) {
        argument = memory.read16(offset);
        addr = memory.read16(argument);
        length = 3;
    }
    else if (addrMode == AddressationMode::IndexedIndirect) {
        argument = memory.read8(offset);
        addr = memory.read16(argument + registers.X);
        length = 2;
    }
    else if (addrMode == AddressationMode::IndirectIndexed) {
        argument = memory.read8(offset);
        Address base = memory.read16(argument);
        addr = base + registers.Y;
        crossPage = (base & 255) != (addr & 255);
        length = 2;
    }
    else {
        throw UnknownAddressModeException{};
    }
    cycles = CyclesByOpcode[opcode];
    switch(opcode) {
    case 0x7D: case 0x79: case 0x71: case 0x3D: case 0x39: case 0x31: case 0xDD: case 0xD9: case 0xD1: case 0x5D: case 0x59: case 0x51:
    case 0xBD: case 0xB9: case 0xB1: case 0xBE: case 0xBC: case 0x1D: case 0x19: case 0x11: case 0xFD: case 0xF9: case 0xF1: {
        if(crossPage) cycles += 1;
        break;
    }
    }
    return Instruction{&memory, val8, addr, argument, length, cycles, addrMode, offset - 1, opcode};
}

CPU::CPU(Memory &_memory, PPU& _ppu, EventQueue& _eventQueue, Logger* _logger)
    : syncTimePoint{}, _registers{}, memory{_memory}, ppu{_ppu}, eventQueue{_eventQueue}, logger{_logger}, instructionCounter{0} {
    // initializing PC with address from Reset Vector
    registers().PC = memory.read16(ResetVectorAddress);
}

void CPU::run() {
    // PARTY HARD
    while(true) {
        exec();
    };
}

void CPU::exec() {
    auto ppuFrameBefore = ppu.currentFrame();
    Instruction instruction = fetchInstruction();
    u8 cyclesBefore = instruction.cycles - 1;
    emulateCycles([this, &instruction]() { return instruction.cycles - 1;  }, false);
    emulateCycles([this, &instruction, cyclesBefore]() { executeInstruction(instruction); return instruction.cycles - cyclesBefore; }, true);
    auto ppuFrameAfter = ppu.currentFrame();
    if(ppuFrameBefore != ppuFrameAfter) _frameSync();
}

Instruction CPU::fetchInstruction() {
    Address offset = registers().PC;
    u8 opcode = memory.read8(offset);
    AddressationMode addrMode;
    Instruction instruction;

    try {
        addrMode = AddrModesByOpcode[opcode];
        instruction = makeInstruction(*this, addrMode, offset, opcode);
    }
    catch (UnknownOpcodeException) {
        if(logger) logger->log(LogLevel::Error, "CPU::step() - unknown opcode " + std::to_string(opcode));
        throw UnknownOpcodeException{};
    }
    catch (UnknownAddressModeException) {
        if(logger) logger->log(LogLevel::Error, "CPU::step() - unknown address mode " + std::to_string((int)addrMode));
        throw UnknownOpcodeException{};
    }
    return instruction;
}

// returns number of cycles instruction elapsed
u8 CPU::executeInstruction(Instruction& instruction) {
    Registers& regs = registers();
    u16 nextBranchAddress = 0;          // set by branching operation
    u16 nextUnconditionalAddress = 0;   // set by any other operation
    switch(instruction.opcode) {
    // ADC
    case 0x69: case 0x65: case 0x75: case 0x6D: case 0x7D: case 0x79: case 0x61: case 0x71: {
        u16 res = regs.A + instruction.val8() + regs.carry();
        // carry can be detected if result is smaller than the first term(as technically we summ only positive numbers)
        // overflow flag is set for a + b = c, if a and b have the same sign, and c has other
        // ~(regs.A ^ adding) will will evaluate to true, if both have same sign, and regs.A ^ res - if both have different signs
        regs.setZero((res & 0xFF) == 0).setNegative(res).setCarry(res > 0xFF).setOverflow((~(regs.A ^ instruction.val8()))&(regs.A ^ res)&0x80);
        regs.A = res & 0xFF;
        break;
    }
    // AND
    case 0x29: case 0x25: case 0x35: case 0x2D: case 0x3D: case 0x39: case 0x21: case 0x31:
        regs.A &= instruction.val8();
        regs.setZero(regs.A).setNegative(regs.A);
        break;
    // ASL
    case 0x0A: case 0x06: case 0x16: case 0x0E: case 0x1E: {
        // accumulator
        if (instruction.opcode == 0x0A) {
            regs.setCarry(regs.A & 0b10000000);
            regs.A <<= 1;
            regs.setZero(regs.A).setNegative(regs.A);
        }
        // memory
        else {
            regs.setCarry(instruction.val8() & 0b10000000);
            u8 res = instruction.val8() << 1;
            memory.write8(instruction.address, res);
            regs.setZero(res).setNegative(res);
        }
        break;
    }
    // BCC
    case 0x90: if(!regs.carry()) nextBranchAddress = instruction.address; break;
    // BCS
    case 0xB0: if(regs.carry()) nextBranchAddress = instruction.address; break;
    // BEQ
    case 0xF0: if(regs.zero()) nextBranchAddress = instruction.address; break;
    // BIT
    case 0x24: case 0x2C: {
        u8 res = regs.A & instruction.val8();
        // negative and overflow flags are set to value of MEMORY bits, zero - to result's
        regs.setZero(res).setNegative(instruction.val8()).setOverflow(instruction.val8() & 0b01000000);
        break;
    }
    // BMI
    case 0x30: if(regs.negative()) nextBranchAddress = instruction.address; break;
    // BNE
    case 0xD0: if(!regs.zero()) nextBranchAddress = instruction.address; break;
    // BPL
    case 0x10: if(!regs.negative()) nextBranchAddress = instruction.address; break;
    // BRK
    case 0x00: {
        interrupt(InterruptType::BRK, regs.PC + instruction.length);
        nextUnconditionalAddress = InterruptVectorAddress;
        break;
    }
    // BVC
    case 0x50: if(!regs.overflow()) nextBranchAddress = instruction.address; break;
    // BVS
    case 0x70: if(regs.overflow()) nextBranchAddress = instruction.address; break;
    // CLC
    case 0x18: regs.setCarry(false); break;
    // CLD
    case 0xD8: regs.setDecimal(false); break;
    // CLI
    case 0x58: regs.setInterruptDisable(false); break;
    // CLV
    case 0xB8: regs.setOverflow(false); break;
    // CMP
    case 0xC9: case 0xC5: case 0xD5: case 0xCD: case 0xDD: case 0xD9: case 0xC1: case 0xD1: {
        u8 res = regs.A - instruction.val8();
        regs.setCarry(regs.A >= instruction.val8()).setZero(regs.A == instruction.val8()).setNegative(res);
        break;
    }
    // CPX
    case 0xE0: case 0xE4: case 0xEC: {
        u8 res = regs.X - instruction.val8();
        regs.setCarry(regs.X >= instruction.val8()).setZero(regs.X == instruction.val8()).setNegative(res);
        break;
    }
    // CPY
    case 0xC0: case 0xC4: case 0xCC: {
        u8 res = regs.Y - instruction.val8();
        regs.setCarry(regs.Y >= instruction.val8()).setZero(regs.Y == instruction.val8()).setNegative(res);
        break;
    }
    // DEC
    case 0xC6: case 0xD6: case 0xCE: case 0xDE: {
        u8 res = instruction.val8() - 1;
        regs.setZero(res).setNegative(res);
        memory.write8(instruction.address, res);
        break;
    }
    // DEX
    case 0xCA: regs.X--; regs.setZero(regs.X).setNegative(regs.X); break;
    // DEY
    case 0x88: regs.Y--; regs.setZero(regs.Y).setNegative(regs.Y);  break;
    // EOR
    case 0x49: case 0x45: case 0x55: case 0x4D: case 0x5D: case 0x59: case 0x41: case 0x51:
        regs.A ^= instruction.val8();
        regs.setZero(regs.A).setNegative(regs.A);
        break;
    // INC
    case 0xE6: case 0xF6: case 0xEE: case 0xFE: {
        u8 res = instruction.val8() + 1;
        regs.setZero(res).setNegative(res);
        memory.write8(instruction.address, res);
        break;
    }
    // INX
    case 0xE8: regs.X++; regs.setZero(regs.X).setNegative(regs.X); break;
    // INY
    case 0xC8: regs.Y++; regs.setZero(regs.Y).setNegative(regs.Y); break;
    // JMP
    case 0x4C: case 0x6C:
        // case of JMP indirect 6502 bug
        if((instruction.opcode == 0x6C) && ((instruction.argument & 0xff) == 0xff)) {
            instruction.address = (memory.read8(instruction.argument & 0xFF00) << 8) + memory.read8(instruction.argument);
        }
        nextUnconditionalAddress = instruction.address;
        break;
    // JSR
    case 0x20:
        // push return address (minus one) on to the stack
        push((u16)(instruction.offset + instruction.length - 1));
        nextUnconditionalAddress = instruction.address;
        break;
    // LDA
    case 0xA9: case 0xA5: case 0xB5: case 0xAD: case 0xBD: case 0xB9: case 0xA1: case 0xB1:
        regs.A = instruction.val8();
        regs.setZero(regs.A).setNegative(regs.A);
        break;
    // LDX
    case 0xA2: case 0xA6: case 0xB6: case 0xAE: case 0xBE:
        regs.X = instruction.val8();
        regs.setZero(regs.X).setNegative(regs.X);
        break;
    // LDY
    case 0xA0: case 0xA4: case 0xB4: case 0xAC: case 0xBC:
        regs.Y = instruction.val8();
        regs.setZero(regs.Y).setNegative(regs.Y);
        break;
    // LSR
    case 0x4A: case 0x46: case 0x56: case 0x4E: case 0x5E: {
        // accumulator
        if(instruction.opcode == 0x4A) {
            regs.setCarry(regs.A & 0b1);
            regs.A >>= 1;
            regs.setZero(regs.A).setNegative(regs.A);
        }
        // memory
        else {
            u8 res = instruction.val8() >> 1;
            regs.setCarry(instruction.val8() & 0b1);
            memory.write8(instruction.address, res);
            regs.setZero(res).setNegative(res);
        }
        break;
    }
    // NOP
    case 0xEA: break;
    // ORA
    case 0x09: case 0x05: case 0x15: case 0x0D: case 0x1D: case 0x19: case 0x01: case 0x11:
        regs.A |= instruction.val8();
        regs.setZero(regs.A).setNegative(regs.A);
        break;
    // PHA
    case 0x48: push(regs.A); break;
    // PHP
    // ??????????? SHOULD I SET B FLAG ??????????????????
    case 0x08: regs.setBFlag(true); push(regs.P); break;
    // PLA
    case 0x68: regs.A = top8(); pop8(); regs.setZero(regs.A).setNegative(regs.A); break;
    // PLP
    case 0x28: regs.P = top8(); pop8(); break;
    // ROL
    case 0x2A: case 0x26: case 0x36: case 0x2E: case 0x3E: {
        // accumulator
        if (instruction.opcode == 0x2A) {
            u8 res = (regs.A << 1) | regs.carry();
            regs.setCarry(regs.A & 0b10000000);
            regs.A = res;
            regs.setZero(regs.A).setNegative(regs.A);
        }
        // memory
        else {
            u8 res = (instruction.val8() << 1) | regs.carry();
            regs.setCarry(instruction.val8() & 0b10000000);
            memory.write8(instruction.address, res);
            regs.setZero(res).setNegative(res);
        }
        break;
    }
    // ROR
    case 0x6A: case 0x66: case 0x76: case 0x6E: case 0x7E: {
        // accumulator
        if(instruction.opcode == 0x6A) {
            u8 res = (regs.A >> 1) | (regs.carry() << 7);
            regs.setCarry(regs.A & 1);
            regs.A = res;
            regs.setZero(regs.A).setNegative(regs.A);
        }
        // memory
        else {
            u8 res = (instruction.val8() >> 1) | (regs.carry() << 7);
            regs.setCarry(instruction.val8() & 1);
            memory.write8(instruction.address, res);
            regs.setZero(res).setNegative(res);
        }
        break;
    }
    // RTI
    case 0x40: regs.P = top8(); pop8(); nextUnconditionalAddress = top16(); pop16(); break;
    // RTS
    case 0x60: nextUnconditionalAddress = top16() + 1; pop16(); break;
    // SBC
    case 0xE9: case 0xE5: case 0xF5: case 0xED: case 0xFD: case 0xF9: case 0xE1: case 0xF1: {
        u16 res = regs.A - instruction.val8() - !(regs.carry());
        // carry flag is CLEARED if carry occures
        // overflow flag is set the same way, as in ADC
        regs.setZero((res & 0xFF) == 0).setNegative(res).setCarry(!(res & 0x100)).setOverflow((regs.A ^ res)&(~instruction.val8() ^ res)&0x80);
        regs.A = res & 0xFF;
        break;
    }
    // SEC
    case 0x38: regs.setCarry(true); break;
    // SED
    case 0xF8: regs.setDecimal(true); break;
    // SEI
    case 0x78: regs.setInterruptDisable(true); break;
    // STA
    case 0x85: case 0x95: case 0x8D: case 0x9D: case 0x99: case 0x81: case 0x91:
        memory.write8(instruction.address, regs.A);
        break;
    // STX
    case 0x86: case 0x96: case 0x8E:
        memory.write8(instruction.address, regs.X);
        break;
    // STY
    case 0x84: case 0x94: case 0x8C:
        memory.write8(instruction.address, regs.Y);
        break;
    // TAX
    case 0xAA: regs.X = regs.A; regs.setZero(regs.X).setNegative(regs.X); break;
    // TAY
    case 0xA8: regs.Y = regs.A; regs.setZero(regs.Y).setNegative(regs.Y); break;
    // TSX
    case 0xBA: regs.X = regs.S; regs.setZero(regs.X).setNegative(regs.X); break;
    // TXA
    case 0x8A: regs.A = regs.X; regs.setZero(regs.A).setNegative(regs.A); break;
    // TXS
    case 0x9A: regs.S = regs.X; break;
    // TYA
    case 0x98: regs.A = regs.Y; regs.setZero(regs.A).setNegative(regs.A); break;
    default:
        // unknown opcode is considered to be NOP
        if(logger) logger->log(LogLevel::Warning, "Unknown opcode " + std::to_string(instruction.opcode) + ". Can it be NOP?");
        instruction.cycles = 2; break;
    }
    // IT SHOULD BE HERE: trying to not trigger unnecessary read operation
#ifdef DEBUG
    if(logger) logger->log(LogLevel::Debug, "[" + std::to_string(instructionCounter) + "][PC: " + numToHexStr(regs.PC, 4) + "]:" + getPrettyInstruction(instruction.opcode, instruction.addrMode, instruction.offset, instruction));
#endif
    if (nextUnconditionalAddress) regs.PC = nextUnconditionalAddress;
    else if (nextBranchAddress) {
        regs.PC = nextBranchAddress;
        // add 1 to cycles if nextBranchAddress
        ++instruction.cycles;
        // if this page != nextBranchAddressPage add one more(page crossing)
        if((instruction.offset & 255) != (nextBranchAddress & 255)) ++instruction.cycles;
    }
    else regs.PC += instruction.length;
    instructionCounter++;
    return instruction.cycles;
}

Serialization::BytesCount CPU::serialize(std::string &buf) {
    u64 syncTimePointNum = std::chrono::time_point_cast<std::chrono::nanoseconds>(syncTimePoint).time_since_epoch().count();
    auto& regs = registers();
    return Serialization::Serializer::serializeAll(buf, &syncTimePointNum, &regs.A, &regs.X, &regs.Y, &regs.PC, &regs.S, &regs.P,
                                                   &memory.get(), &instructionCounter);
}

Serialization::BytesCount CPU::deserialize(const std::string &buf, Serialization::BytesCount offset) {
    auto& regs = registers();
    u64 syncTimePointNum;
    auto memWr = wrapArr(memory.get());
    auto res = Serialization::Deserializer::deserializeAll(buf, offset, &syncTimePointNum, &regs.A, &regs.X, &regs.Y, &regs.PC, &regs.S, &regs.P,
                                                   &memWr, &instructionCounter);
    syncTimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds>(std::chrono::nanoseconds(syncTimePointNum));
    return res;
}

/*
    Accepts function f, that accepts 0 args and returns the number of cycles it took to emulate it.
    It should execute approximately the time of cycle * f return value;
*/
void CPU::emulateCycles(std::function<int(void)> f, bool processEvents) {
    int cycles = f();
    // PPU works on 3*CPUFrequency
    for(int i = 0; i < cycles * 3; ++i) {
        ppu.emulateCycle();
    }
    if(processEvents && !eventQueueEmpty()) _processEventQueue();
}

void CPU::interrupt(InterruptType intType, Address nextPC) {
    // NMI should be processed
    if(registers().interruptDisable() && intType != InterruptType::NMI) return;
    registers().setBFlag(intType == InterruptType::BRK);    // only BRK sets B flag
    push((u16)nextPC).push(registers().P);
    registers().setInterruptDisable(true);
    registers().PC = intType == InterruptType::NMI ? memory.read16(NonMaskableInterruptVectorAddress) : memory.read8(InterruptVectorAddress);
}

// using a frame as syncrhronization unit
void CPU::_frameSync() {
    static const u32 MaxFrameDurationNs = 1000000000 / 60;
    auto curTimePoint = std::chrono::high_resolution_clock::now();
    auto sleepDuration = std::chrono::nanoseconds(MaxFrameDurationNs - (curTimePoint - syncTimePoint).count());
    std::this_thread::sleep_for(std::chrono::nanoseconds(sleepDuration));
    syncTimePoint = std::chrono::high_resolution_clock::now();
}

void CPU::_processEventQueue() {
    auto& events = eventQueue.get();
    while(!events.empty()) {
        EventType eventType = events.front();
        events.pop();
        switch(eventType) {
        case EventType::InterruptNMI: interrupt(InterruptType::NMI, registers().PC); break;
        case EventType::OAMDMAWrite: oamDmaWrite(); break;
        default: {
            if(logger) logger->log(LogLevel::Error, "CPU::_processEventQueue(): unknown CPU event type " + std::to_string((int)eventType));
            throw UnknownCPUEventException{};
        }
        }
    }
}

/*
    Sequentially writes 256 bytes of data from CPU page $XX00-$XXFF to the internal PPU's OAM memory.
    It should take exactly 512 CPU cycles.
*/
void CPU::oamDmaWrite() {
    // I hope that nothing bad will happen if I read from OAMDMA
    u8 page = ppu.accessPPURegisters().readOamdma();
    auto& OAM = ppu.getOAM();
    u8 startOAMAddr = ppu.accessPPURegisters().readOamaddr();
    for(int i = 0; i < 0x100; ++i) {
        Address addr = (page << 8) + i;
        // this will make address cyclic(256 bytes)
        u8 oamAddr = startOAMAddr + i;
        u8 val = 0;
        emulateCycles([this, &val, addr] {
            val = memory.read8(addr);
            return 1;
        }, false);
        emulateCycles([this, &OAM, i, startOAMAddr, oamAddr, val]() {
            OAM[oamAddr] = val;
#ifdef DEBUG
            if (logger) logger->log(LogLevel::Debug, "[OAMDMA][" + std::to_string(instructionCounter) + "]: write of " + std::to_string(val) + " to OAM[" + std::to_string(oamAddr) + "]");
#endif
            // each such operation consumes 2 CPU cycles
            ++instructionCounter;
            return 1;
        }, true);
    }
}

std::string getPrettyInstruction(u8 opcode, AddressationMode addrMode, Address curAddress, Instruction instruction) {

    std::string hexVal8 = instruction.hasVal8() ? numToHexStr(instruction.val8(), 2) : "UNKNOWN";
    std::string hexAddr8 = numToHexStr(instruction.address, 2);
    std::string hexAddr16 = numToHexStr(instruction.address, 4);
    std::string curHexAddr8 = numToHexStr(instruction.argument, 2);;
    std::string curHexAddr16 = numToHexStr(instruction.argument, 4);;
    i16 relAddress = instruction.address - curAddress;

    std::string pretty;

    switch(addrMode) {
    case AddressationMode::Implied: pretty = ""; break;
    case AddressationMode::Accumulator: pretty = "A"; break;
    case AddressationMode::Immediate: pretty = "#" + hexVal8; break;
    case AddressationMode::ZeroPage: pretty = "$" + hexAddr8; break;
    case AddressationMode::ZeroPageX: pretty = "$" + curHexAddr8 + ", X => " + hexAddr16; break;
    case AddressationMode::ZeroPageY: pretty = "$" + curHexAddr8 + ", Y => " + hexAddr16; break;
    case AddressationMode::Relative: pretty = "*" + std::string(relAddress > 0 ? "+" : "") + std::to_string(relAddress); break;
    case AddressationMode::Absolute: pretty = "$" + hexAddr16; break;
    case AddressationMode::AbsoluteX: pretty = "$" + curHexAddr16 + ", X => " + hexAddr16; break;
    case AddressationMode::AbsoluteY: pretty = "$" + curHexAddr16 + ", Y => " + hexAddr16; break;
    case AddressationMode::Indirect: pretty = "($" + curHexAddr16 + ")"; break;
    case AddressationMode::IndexedIndirect: pretty = "($" + curHexAddr8 + ", X) => " + hexAddr16; break;
    case AddressationMode::IndirectIndexed: pretty = "($" + curHexAddr8 + "), Y => " + hexAddr16; break;
    }

    return OpcodeToString[opcode] + " " + pretty;
}

