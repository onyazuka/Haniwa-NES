#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <memory>

typedef int8_t i8;
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

