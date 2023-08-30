#pragma once

#include <immintrin.h>
#include "memory.h"

typedef struct vec256i vec256i;
struct vec256i {
    _Alignas(BLOCK_32) u64 data[4];
};
