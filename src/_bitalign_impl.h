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
    if (0) {printf("%d-->%d\n", (SHIFT), (COMMON));}               \
    if ((COMMON) >= res.common_bits) {                             \
        int _shift = (SHIFT);                                      \
        if ((COMMON) > res.common_bits || _shift < res.shift_by) { \
            res.common_bits = (COMMON);                            \
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
        int overlap = N * WORD_BIT;
        for (int b_start = 0; b_start < N; b_start++, overlap -= WORD_BIT) {
            if (overlap < res.common_bits) {
                break;
            }
            // compare buffer[0:N-b_start] to b[b_start:N]
            int bi = b_start, ai = 0;
            int common = overlap;
            for (; bi < N; ai++, bi++) {
                common -= POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT(common, WORD_BIT * b_start);
        }
        overlap = (N - 1) * WORD_BIT;
        for (int a_start = 1; a_start < N; a_start++, overlap -= WORD_BIT) {
            if (overlap < res.common_bits) {
                break;
            }
            // compare buffer[a_start:N] to b[0:N-a_start]
            int ai = a_start, bi = 0;
            int common = overlap;
            for (; ai < N; ai++, bi++) {
                common -= POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT(common, (-WORD_BIT) * a_start);
        }
    }
    // Remaining Iterations: now buffer has N+1 words.
    // buffer[0] and buffer[N] are only partial words.
    for (int iteration = 1; iteration < WORD_BIT; iteration++) {
        MANGLE(do_shift)(buffer, N + 1);
        WORD a0mask = SHIFT_FORWARD(WORD_MAX, iteration);
        int overlap = (N - 1) * WORD_BIT + WORD_BIT - iteration;
        for (int b_start = 0; b_start < N; b_start++, overlap -= WORD_BIT) {
            if (overlap < res.common_bits) {
                break;
            }
            // compare buffer[0:N-b_start] to b[b_start:N]
            int common = overlap;
            // buffer[0] is only partially present
            common -= POPCNT(a0mask & (buffer[0] ^ b[b_start]));
            // now the rest: buffer[1:N-b_start] to b[b_start+1:N]
            int ai = 1, bi = b_start + 1;
            for (; bi < N; ai++, bi++) {
                common -= POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT(common, (WORD_BIT) * b_start + iteration);
        }
        WORD aNmask = (WORD)~a0mask;
        overlap = (N - 1) * WORD_BIT + iteration;
        for (int a_start = 1; a_start <= N; a_start++, overlap -= WORD_BIT) {
            if (overlap < res.common_bits) {
                break;
            }
            // only compare buffer[a_start:N] to b[0:N-a_start]
            int common = overlap;
            // buffer[N] is only partially present:
            common -= POPCNT(aNmask & (b[N-a_start] ^ buffer[N]));
            // now the rest: buffer[a_start:N+1] with b[0:N+1-a_start]
            int ai = a_start, bi = 0;
            for (; ai < N; ai++, bi++) {
                common -= POPCNT(buffer[ai] ^ b[bi]);
            }
            UPDATE_RESULT(common, (-WORD_BIT) * a_start + iteration);
        }
    }
    return res;
}

#undef UPDATE_RESULT
