#pragma once
#include "mapper0.hpp"

Uptr<MapperInterface> makeMapper(int number, ROM& rom, Logger* logger);
