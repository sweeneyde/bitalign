#if !defined(WORD) || !defined(POPCNT) || !defined(MANGLE) \
    || !defined(WORD_MAX) || !defined(LAST_BIT_AND_SHIFTED_REST) \
    || !defined(SHIFT_FORWARD) || !defined(WORD_BIT)
#error "bitalign.h included incorrectly."
#endif

static void
MANGLE(do_shift)(WORD *buf, int len)
{
    WORD prev = 0;
    for (int i = 0; i < len; i++) {
        WORD old = buf[i];
        buf[i] = LAST_BIT_AND_SHIFTED_REST(prev, old);
        prev = old;
    }
}

#define UPDATE_RESULT(COMMON, SHIFT) do {                          \
    int _common = (COMMON);                                        \
    if (0) {printf("%d-->%d\n", (SHIFT), (COMMON));}               \
    if (_common >= res.common_bits) {                              \
        int _shift = (SHIFT);                                      \
        if (_common > res.common_bits || _shift < res.shift_by) {  \
            res.common_bits = _common;                             \
            res.shift_by = _shift;                                 \
        }                                                          \
    }                                                              \
} while (0)


struct bitalign_result
MANGLE(bitalign_impl)(const WORD *a, const WORD *b, int N, WORD *buffer)
{
    // buffer must have N+1 words to work with.
    struct bitalign_result res = {.shift_by = 0, .common_bits = -1};
    assert(N > 0);
    assert(N < INT_MAX / WORD_BIT / 2);
    memcpy(buffer, a, N * sizeof(WORD));
    buffer[N] = 0;
    {
        // Iteration 0: No bit-shifts yet. Buffer has N words.
        for (int b_start = 0; b_start < N; b_start++) {
            // compare buffer[0:N-b_start] to b[b_start:N]
            int bi = b_start, ai = 0;
            int diff = 0;
            for (; bi < N; ai++, bi++) {
                diff += POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT((N - b_start) * WORD_BIT - diff,\
                          WORD_BIT * b_start);
        }
        for (int a_start = 1; a_start < N; a_start++) {
            // compare buffer[a_start:N] to b[0:N-a_start]
            int ai = a_start, bi = 0;
            int diff = 0;
            for (; ai < N; ai++, bi++) {
                diff += POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT((N - a_start) * WORD_BIT - diff,\
                          (-WORD_BIT) * a_start);
        }
    }
    // Remaining Iterations: now buffer has N+1 words.
    // buffer[0] and buffer[N] are only partial words.
    for (int iteration = 1; iteration < WORD_BIT; iteration++) {
        MANGLE(do_shift)(buffer, N + 1);
        WORD a0mask = SHIFT_FORWARD(WORD_MAX, iteration);
        for (int b_start = 0; b_start < N; b_start++) {
            // compare buffer[0:N-b_start] to b[b_start:N]
            // buffer[0] is only partially present, so first
            // only compare buffer[1:N-b_start] to b[b_start+1:N]
            int ai = 1, bi = b_start + 1;
            int diff = 0;
            for (; bi < N; ai++, bi++) {
                diff += POPCNT(buffer[ai] ^ b[bi]);
            }
            int common = (N - b_start - 1) * WORD_BIT - diff;
            // now add in the partial word
            common += POPCNT(a0mask & ~(buffer[0] ^ b[b_start]));
            UPDATE_RESULT(common, (WORD_BIT) * b_start + iteration);
        }
        WORD aNmask = (WORD)~a0mask;
        for (int a_start = 1; a_start <= N; a_start++) {
            // compare buffer[a_start:N+1] with b[0:N+1-a_start]
            // buffer[N] is only partially present, so first
            // only compare buffer[a_start:N] to b[0:N-a_start]
            int ai = a_start, bi = 0;
            int diff = 0;
            for (; ai < N; ai++, bi++) {
                diff += POPCNT(buffer[ai] ^ b[bi]);
            }
            int common = (N - a_start) * WORD_BIT - diff;
            // now add in the partial word
            common += POPCNT(aNmask & ~(b[N-a_start] ^ buffer[N]));
            UPDATE_RESULT(common, (-WORD_BIT) * a_start + iteration);
        }
    }
    return res;
}

#undef UPDATE_RESULT
