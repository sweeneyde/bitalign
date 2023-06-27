from array import array
from random import randbytes
from time import perf_counter as now

from bitalign import (
    bitalign_8_msb as f8,
    bitalign_16_msb as f16,
    bitalign_32_msb as f32,
    bitalign_64_msb as f64,
)

def make_cases(bits):
    b = randbytes(bits // 8)
    return [array(code, b) for code in 'BHIQ']

def main(outer_loops=50, inner_loops=50, bits=2048):
    print(f"Benchmarking with {bits} bits...")
    assert bits % 64 == 0
    t8, t16, t32, t64 = 0.0, 0.0, 0.0, 0.0
    inner = range(inner_loops)

    for _ in range(outer_loops):
        a8, a16, a32, a64 = make_cases(bits)
        b8, b16, b32, b64 = make_cases(bits)

        t0 = now()
        for _ in inner:
            f8(a8, b8)
        t8 += now() - t0

        t0 = now()
        for _ in inner:
            f16(a16, b16)
        t16 += now() - t0

        t0 = now()
        for _ in inner:
            f32(a32, b32)
        t32 += now() - t0

        t0 = now()
        for _ in inner:
            f64(a64, b64)
        t64 += now() - t0

        print(".", end='', flush=True)

    print()
    loops = outer_loops * inner_loops
    print(f'bitalign_8_xxx:  {t8/loops*1e6:>5.1f}us')
    print(f'bitalign_16_xxx: {t16/loops*1e6:>5.1f}us')
    print(f'bitalign_32_xxx: {t32/loops*1e6:>5.1f}us')
    print(f'bitalign_64_xxx: {t64/loops*1e6:>5.1f}us')


if __name__ == "__main__":
    main()
