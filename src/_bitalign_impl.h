#if !defined(WORD) || !defined(POPCNT) || !defined(MANGLE) \
    || !defined(WORD_MAX) || !defined(LAST_BIT_AND_SHIFTED_REST) \
    || !defined(SHIFT_FORWARD) || !defined(WORD_BIT)
#error "bitalign.h included incorrectly."
#endif

/*
Implementation strategy:
To avoid having to shift the "a" buffer by one bit at every step, do
everything we can with that shift position, and only then shift it by
one bit.

For an example, suppose we had 4-bit words, and we wanted to
compare a pair of 12 bit strings.

  iteration=0:
  buffer:               aaaa aaaa aaaa ____
  b_start=0             bbbb bbbb bbbb                 shift_by=0
  b_start=1        bbbb bbbb bbbb                      shift_by=4
  b_start=2   bbbb bbbb bbbb                           shift_by=8
  a_start=1                  bbbb bbbb bbbb            shift_by=-4
  a_start=2                       bbbb bbbb bbbb       shift_by=-8
  a_start=3                            bbbb bbbb bbbb  shift_by=-12

  iteration=1:
  buffer:               _aaa aaaa aaaa a___
  b_start=0             bbbb bbbb bbbb                 shift_by=1
  b_start=1        bbbb bbbb bbbb                      shift_by=5
  b_start=2   bbbb bbbb bbbb                           shift_by=9
  a_start=1                  bbbb bbbb bbbb            shift_by=-3
  a_start=2                       bbbb bbbb bbbb       shift_by=-7
  a_start=3                            bbbb bbbb bbbb  shift_by=-11

  iteration=2:
  buffer:               __aa aaaa aaaa aa__
  b_start=0             bbbb bbbb bbbb                 shift_by=2
  b_start=1        bbbb bbbb bbbb                      shift_by=6
  b_start=2   bbbb bbbb bbbb                           shift_by=10
  a_start=1                  bbbb bbbb bbbb            shift_by=-2
  a_start=2                       bbbb bbbb bbbb       shift_by=-6
  a_start=3                            bbbb bbbb bbbb  shift_by=-10

  iteration=3:
  buffer:               ___a aaaa aaaa aaa_
  b_start=0             bbbb bbbb bbbb                 shift_by=3
  b_start=1        bbbb bbbb bbbb                      shift_by=7
  b_start=2   bbbb bbbb bbbb                           shift_by=11
  a_start=1                  bbbb bbbb bbbb            shift_by=-1
  a_start=2                       bbbb bbbb bbbb       shift_by=-5
  a_start=3                            bbbb bbbb bbbb  shift_by=-9
*/

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
    for (int iteration = 0; iteration < WORD_BIT; iteration++) {
        if (iteration > 0) {
            MANGLE(do_shift)(buffer, N + 1);
        }
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
