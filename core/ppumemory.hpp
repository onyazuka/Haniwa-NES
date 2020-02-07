#pragma once
#include <array>
#include "core/common.hpp"
#include "core/mappers/mappers.hpp"

class PPUMemory {
public:
    PPUMemory(MapperInterface& _mapper);
    u8 read(Address address);
    PPUMemory& write(Address address, u8 val);
private:
    Address _fixAddress(Address address);

    std::array<u8, 0x4000> memory;
    MapperInterface& mapper;
};
