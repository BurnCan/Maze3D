#pragma once

#include <cstdint>

enum Direction : uint8_t {
    North = 1 << 0,
    East  = 1 << 1,
    South = 1 << 2,
    West  = 1 << 3
};
