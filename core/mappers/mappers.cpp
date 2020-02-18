#include "mappers.hpp"

Uptr<MapperInterface> makeMapper(int number, ROM& rom, Logger* logger) {
    switch(number) {
    case 0:
        return std::make_unique<Mapper0>(Mapper0(rom, logger));
    default:
        throw InvalidMapperException{};
    }
}