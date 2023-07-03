#include <stdint.h>
#include <stddef.h>

struct bitalign_result {
    int shift_by;
    int common_bits;
};

typedef struct bitalign_result (*implfunc)(void *, void *, int, void *);

struct bitalign_result bitalign_impl_8lsb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_16lsb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_32lsb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_64lsb(void *, void *, int, void *);

struct bitalign_result bitalign_impl_8msb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_16msb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_32msb(void *, void *, int, void *);
struct bitalign_result bitalign_impl_64msb(void *, void *, int, void *);

typedef void (*implfunc_multi)(void *, void *, size_t, int, void *, struct bitalign_result *);

void bitalign_multi_impl_8lsb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_16lsb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_32lsb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_64lsb(void *, void *, size_t, int, void *, struct bitalign_result *);

void bitalign_multi_impl_8msb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_16msb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_32msb(void *, void *, size_t, int, void *, struct bitalign_result *);
void bitalign_multi_impl_64msb(void *, void *, size_t, int, void *, struct bitalign_result *);
