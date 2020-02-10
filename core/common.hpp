#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include "debug/debug.hpp"

typedef int8_t i8;
typedef int16_t i16;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
template<std::size_t N>
using Bytes = std::array<u8, N>;
using DinBytes = std::vector<u8>;
template<typename T>
using Uptr = std::unique_ptr<T>;
template<typename T>
using Sptr = std::shared_ptr<T>;

typedef i8 RelativeOffset;
typedef u8 ZeroPageAddress;
typedef u16 Address;

const int PAGE_SIZE = 0x100;

const std::vector<u8> SupportedMappers{0};

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

enum class InterruptType {
    BRK,
    NMI,
    IRQ,
    RESET
};

enum class Mirroring {
    Horizontal,
    Vertical
};

template<typename T>
T ROL(T val, u8 sz) {
    return (val << sz) + (val >> (sizeof(T) * 8 - sz));
}

template<typename T>
T ROR(T val, u8 sz) {
    return (val >> sz) + (val << (sizeof(T) * 8 - sz));
}

template<typename T>
bool contains(const std::vector<T>& v, T val) {
    return std::find(v.begin(), v.end(), val) != v.end();
}

// we need next 2 functions to minimize error count using it everywhere
template<typename Cont>
u16 read16Contigous(const Cont& bytes, Address offset) {
    return bytes[offset] + (bytes[offset + 1] << 8);
}

template<typename Cont>
void write16Contigous(Cont& bytes, Address offset, u16 val) {
    bytes[offset] = (u8)(val & 0xff);
    bytes[offset + 1] = (u8)((val & 0xff00) >> 8);
}

bool isInPRGROM(Address address);

