#include "core/include/mappers/mappers.hpp"

Sptr<MapperInterface> makeMapper(int number, ROM& rom, Logger* logger) {
    switch(number) {
    case 0:
        return std::make_shared<Mapper0>(Mapper0(rom, logger));
    case 1:
        return std::make_shared<Mapper1>(Mapper1(rom, logger));
    default:
        throw InvalidMapperException{};
    }
}
