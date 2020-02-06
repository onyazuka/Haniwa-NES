#include "common.hpp"

bool isInPRGROM(Address address) {
    return address >= 0x8000;
}
