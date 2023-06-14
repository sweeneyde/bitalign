#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "./_bitalign.h"
#include "./_popcount_impl.h"

// LSB Stored first --------------------------------------------------

#define SHIFT_FORWARD(w, i) ((w) << (i))
#define LAST_BIT_AND_SHIFTED_REST(prev, rest) \
    (((prev) >> (WORD_BIT - 1)) | (rest << 1))

#define MANGLE(name)   name##_8lsb
#define WORD           uint8_t
#define WORD_BIT       8
#define POPCNT(x)      popcount8(x)
#define WORD_MAX       UINT8_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_16lsb
#define WORD           uint16_t
#define WORD_BIT       16
#define POPCNT(x)      popcount16(x)
#define WORD_MAX       UINT16_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_32lsb
#define WORD           uint32_t
#define WORD_BIT       32
#define POPCNT(x)      popcount32(x)
#define WORD_MAX       UINT32_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_64lsb
#define WORD           uint64_t
#define WORD_BIT       64
#define POPCNT(x)      popcount64(x)
#define WORD_MAX       UINT64_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#undef SHIFT_FORWARD
#undef LAST_BIT_AND_SHIFTED_REST

// MSB Stored first --------------------------------------------------

#define SHIFT_FORWARD(w, i) ((w) >> (i))
#define LAST_BIT_AND_SHIFTED_REST(prev, rest) \
    (((prev) << (WORD_BIT - 1)) | (rest >> 1))

#define MANGLE(name)   name##_8msb
#define WORD           uint8_t
#define WORD_BIT       8
#define POPCNT(x)      popcount8(x)
#define WORD_MAX       UINT8_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_16msb
#define WORD           uint16_t
#define WORD_BIT       16
#define POPCNT(x)      popcount16(x)
#define WORD_MAX       UINT16_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_32msb
#define WORD           uint32_t
#define WORD_BIT       32
#define POPCNT(x)      popcount32(x)
#define WORD_MAX       UINT32_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#define MANGLE(name)   name##_64msb
#define WORD           uint64_t
#define WORD_BIT       64
#define POPCNT(x)      popcount64(x)
#define WORD_MAX       UINT64_MAX
#include "./_bitalign_impl.h"
#undef MANGLE
#undef WORD
#undef WORD_BIT
#undef POPCNT
#undef WORD_MAX

#undef SHIFT_FORWARD
#undef LAST_BIT_AND_SHIFTED_REST
