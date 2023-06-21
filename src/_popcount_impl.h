
static inline int
popcount32(uint32_t x)
{
#if (defined(__clang__) || defined(__GNUC__)) && (UINT_MAX == UINT32_MAX)
    return __builtin_popcount(x);
#else
    // From https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    x = x - ((x >> 1) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2) & 0x33333333U);
    return ((x + (x >> 4) & 0xF0F0F0FU) * 0x1010101U) >> 24;
#endif
}

static inline int
popcount64(uint64_t x)
{
    // the intrinsic isn't all that beneficial on my machine.
#if 0 && (defined(__clang__) || defined(__GNUC__)) && (ULLINT_MAX == UINT64_MAX)
    return __builtin_popcountll(x);
#else
    // From https://en.wikipedia.org/wiki/Hamming_weight

    const uint64_t m1  = 0x5555555555555555; //binary: 0101...
    const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
    const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
    const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
#endif
}

static inline int
popcount16(uint16_t x)
{
    return popcount32(x);
}

static const uint8_t POP8_TABLE[256] = {
    0, 1, 1, 2,    1, 2, 2, 3,    1, 2, 2, 3,    2, 3, 3, 4,
    1, 2, 2, 3,    2, 3, 3, 4,    2, 3, 3, 4,    3, 4, 4, 5,
    1, 2, 2, 3,    2, 3, 3, 4,    2, 3, 3, 4,    3, 4, 4, 5,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    1, 2, 2, 3,    2, 3, 3, 4,    2, 3, 3, 4,    3, 4, 4, 5,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    3, 4, 4, 5,    4, 5, 5, 6,    4, 5, 5, 6,    5, 6, 6, 7,
    1, 2, 2, 3,    2, 3, 3, 4,    2, 3, 3, 4,    3, 4, 4, 5,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    3, 4, 4, 5,    4, 5, 5, 6,    4, 5, 5, 6,    5, 6, 6, 7,
    2, 3, 3, 4,    3, 4, 4, 5,    3, 4, 4, 5,    4, 5, 5, 6,
    3, 4, 4, 5,    4, 5, 5, 6,    4, 5, 5, 6,    5, 6, 6, 7,
    3, 4, 4, 5,    4, 5, 5, 6,    4, 5, 5, 6,    5, 6, 6, 7,
    4, 5, 5, 6,    5, 6, 6, 7,    5, 6, 6, 7,    6, 7, 7, 8,
};

static inline int
popcount8(uint8_t x)
{
    return POP8_TABLE[x];
}
