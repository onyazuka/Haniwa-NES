#pragma once
#include <cstdint>

typedef int8_t i8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef i8 RelativeOffset;
typedef u8 ZeroPageAddress;
typedef u16 Address;

const int PAGE_SIZE = 0x100;

enum class AddressationMode {
    Implicit,
    Accumulator,
    Immediate,
    ZeroPage,
    ZeroPageX,
    ZeroPageY,
    Relative,
    Absolute,
    AbsoluteX,
    AbsoluteY,
    Indirect,
    IndexedIndirect,
    IndirectIndexed
};

template<typename T>
T ROL(T val, u8 sz) {
    return (val << sz) + (val >> (sizeof(T) * 8 - sz));
}

template<typename T>
T ROR(T val, u8 sz) {
    return (val >> sz) + (val << (sizeof(T) * 8 - sz));
}

