#pragma once
#include "mapper0.hpp"
#include "mapper1.hpp"

Sptr<MapperInterface> makeMapper(int number, ROM& rom, Logger* logger);
