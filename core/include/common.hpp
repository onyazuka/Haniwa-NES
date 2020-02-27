#pragma once
#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include "debug/debug.hpp"
#include "serialize/serializer.hpp"

// uncomment this for debug
#define DEBUG

typedef int8_t i8;
typedef int16_t i16;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
template<std::size_t N>
using Bytes = std::array<u8, N>;
using DinBytes = std::vector<u8>;
template<typename T>
using Uptr = std::unique_ptr<T>;
template<typename T>
using Sptr = std::shared_ptr<T>;

typedef i8 RelativeOffset;
typedef u8 ZeroPageAddress;
typedef u32 Address;

const int PAGE_SIZE = 0x100;
const int PRG_BANK_SIZE = 0x4000;
const int CHR_BANK_SIZE = 0x1000;

const std::vector<u8> SupportedMappers{0,1};

enum class AddressationMode {
    Implied,
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

// aliases
const AddressationMode IML = AddressationMode::Implied;
const AddressationMode ACC = AddressationMode::Accumulator;
const AddressationMode IMM = AddressationMode::Immediate;
const AddressationMode ZP0 = AddressationMode::ZeroPage;
const AddressationMode ZPX = AddressationMode::ZeroPageX;
const AddressationMode ZPY = AddressationMode::ZeroPageY;
const AddressationMode REL = AddressationMode::Relative;
const AddressationMode ABS = AddressationMode::Absolute;
const AddressationMode ABX = AddressationMode::AbsoluteX;
const AddressationMode ABY = AddressationMode::AbsoluteY;
const AddressationMode IND = AddressationMode::Indirect;
const AddressationMode IIR = AddressationMode::IndexedIndirect;
const AddressationMode IIX = AddressationMode::IndirectIndexed;

const std::array<AddressationMode, 256> AddrModesByOpcode = {
    IML, IIR, IML, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
    ABS, IIR, IML, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
    IML, IIR, IML, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
    IML, IIR, IML, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, IND, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
    IMM, IIR, IMM, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPY, ZPY, IML, ABY, IML, ABY, ABX, ABX, ABY, ABY,
    IMM, IIR, IMM, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPY, ZPY, IML, ABY, IML, ABY, ABX, ABX, ABY, ABY,
    IMM, IIR, IMM, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
    IMM, IIR, IMM, IIR, ZP0, ZP0, ZP0, ZP0, IML, IMM, IML, IMM, ABS, ABS, ABS, ABS, REL, IIX, IML, IIX, ZPX, ZPX, ZPX, ZPX, IML, ABY, IML, ABY, ABX, ABX, ABX, ABX,
};

enum class InterruptType {
    BRK,
    NMI,
    IRQ,
    RESET
};

enum class Mirroring {
    Horizontal,
    Vertical,
    OneScreenLower,
    OneScreenUpper,
};

class UnknownMirroringType {};

template<typename T>
struct InvalidRangeException{
    T min;
    T max;
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

template<typename T>
T& setBit(T& val, u8 num) {
    val |= (1 << num);
    return val;
}

template<typename T>
T& setBits(T& val, std::vector<u8> bits) {
    for(auto bitNum : bits) setBit(val, bitNum);
    return val;
}

template<typename T>
T& clearBit(T& val, u8 num) {
    val &= ~((T)(1 << num));
    return val;
}

template<typename T>
T& clearBits(T& val, std::vector<u8> bits) {
    for(auto bitNum : bits) clearBit(val, bitNum);
    return val;
}

u8 reverseByte(u8 b);

template<typename T, std::size_t N>
Serialization::ArrayWrapper<T> wrapArr(std::array<T,N>& arr) {
    return Serialization::ArrayWrapper<T>{arr.data(), arr.size()};
}
