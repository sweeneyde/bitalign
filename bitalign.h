#include <stdint.h>

struct bitalign_result {
    int shift_by;
    int common_bits;
};

struct bitalign_result
bitalign_impl_8lsb(const uint8_t*, const uint8_t*, int N, uint8_t*);
struct bitalign_result
bitalign_impl_16lsb(const uint16_t*, const uint16_t*, int N, uint16_t*);
struct bitalign_result
bitalign_impl_32lsb(const uint32_t*, const uint32_t*, int N, uint32_t*);
struct bitalign_result
bitalign_impl_64lsb(const uint64_t*, const uint64_t*, int N, uint64_t*);

struct bitalign_result
bitalign_impl_8msb(const uint8_t*, const uint8_t*, int N, uint8_t*);
struct bitalign_result
bitalign_impl_16msb(const uint16_t*, const uint16_t*, int N, uint16_t*);
struct bitalign_result
bitalign_impl_32msb(const uint32_t*, const uint32_t*, int N, uint32_t*);
struct bitalign_result
bitalign_impl_64msb(const uint64_t*, const uint64_t*, int N, uint64_t*);

