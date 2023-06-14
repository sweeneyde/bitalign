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
    memcpy(buffer, b, N * sizeof(WORD));
    buffer[N] = 0;
    {
        // Iteration 0: No bit-shifts yet. Buffer has N words.
        for (int a_start = 0; a_start < N; a_start++) {
            // compare a[a_start:N] to buffer[0:N-a_start]
            int ai = a_start, bi = 0;
            int diff = 0;
            for (; ai < N; ai++, bi++) {
                diff += POPCNT(a[ai] ^ buffer[bi]);
            }
            UPDATE_RESULT((N - a_start) * WORD_BIT - diff,\
                          (-WORD_BIT) * a_start);
        }
        for (int b_start = 1; b_start < N; b_start++) {
            // compare a[0:N-b_start] to buffer[b_start:N]
            int bi = b_start, ai = 0;
            int diff = 0;
            for (; bi < N; ai++, bi++) {
                diff += POPCNT(a[ai] ^ buffer[bi]);
            }
            UPDATE_RESULT((N - b_start) * WORD_BIT - diff,\
                          WORD_BIT * b_start);
        }
    }
    // Remaining Iterations: now buffer has N+1 words.
    // buffer[0] and buffer[N] are only partial words.
    for (int iteration = 1; iteration < WORD_BIT; iteration++) {
        MANGLE(do_shift)(buffer, N + 1);
        WORD b0mask = SHIFT_FORWARD(WORD_MAX, iteration);
        for (int a_start = 0; a_start < N; a_start++) {
            // compare a[a_start:N] to buffer[0:N-a_start]
            // buffer[0] is only partially present, so first
            // only compare a[a_start+1:N] to buffer[1:N-a_start]
            int ai = a_start + 1, bi = 1;
            int diff = 0;
            for (; ai < N; ai++, bi++) {
                diff += POPCNT(a[ai] ^ buffer[bi]);
            }
            int common = (N - a_start - 1) * WORD_BIT - diff;
            // now add in the partial word
            common += POPCNT(b0mask & ~(a[a_start] ^ buffer[0]));
            UPDATE_RESULT(common, (-WORD_BIT) * a_start - iteration);
        }
        WORD bNmask = (WORD)~b0mask;
        for (int b_start = 1; b_start <= N; b_start++) {
            // compare a[0:N+1-b_start] with buffer[b_start:N+1]
            // buffer[N] is only partially present, so first
            // only compare a[0:N-b_start] to buffer[b_start:N]
            int ai = 0, bi = b_start;
            int diff = 0;
            for (; bi < N; ai++, bi++) {
                diff += POPCNT(a[ai] ^ buffer[bi]);
            }
            int common = (N - b_start) * WORD_BIT - diff;
            // now add in the partial word
            common += POPCNT(bNmask & ~(a[N-b_start] ^ buffer[N]));
            UPDATE_RESULT(common, WORD_BIT * b_start - iteration);
        }
    }
    return res;
}

#undef UPDATE_RESULT
