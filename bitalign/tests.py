import array
import operator
import platform
import random
import unittest

from bitalign import (
    bitalign_8_lsb,
    bitalign_16_lsb,
    bitalign_32_lsb,
    bitalign_64_lsb,
    bitalign_8_msb,
    bitalign_16_msb,
    bitalign_32_msb,
    bitalign_64_msb,
)


ARRAY_BIT_TO_TYPECODE = {8: 'B', 16: 'H', 32: 'I', 64: 'Q'}

class BitAlignTest(unittest.TestCase):

    def test_basic_8_msb(self):
        (x, y) = bitalign_8_msb(bytes([0b1111_1111, 0b1111_1111]),
                                bytes([0b1111_1111, 0b1111_1111]))
        self.assertEqual((x, y), (0, 16))

        (x, y) = bitalign_8_msb(bytes([0b1111_1111, 0b1111_1111]),
                                bytes([0b0000_0000, 0b0000_0000]))
        self.assertEqual((x, y), (-15, 0))

        (x, y) = bitalign_8_msb(bytes([0b1111_0000, 0b0000_0000]),
                                bytes([0b0000_1111, 0b0000_0000]))
        self.assertEqual((x, y), (4, 12))

        (x, y) = bitalign_8_msb(bytes([0b0000_1111, 0b0000_0000]),
                                bytes([0b0000_0000, 0b1111_0000]))
        self.assertEqual((x, y), (4, 12))

        (x, y) = bitalign_8_msb(bytes([0b1001_0010, 0b0010_0101]),
                                bytes([0b0110_0010, 0b0111_1100]))
        self.assertEqual((x, y), (-4, 10))

        (x, y) = bitalign_8_msb(bytes([0b1101_1110]),
                                bytes([0b1011_0100]))
        self.assertEqual((x, y), (-1, 6))

        (x, y) = bitalign_8_msb(bytes([0b0000_0110]),
                                bytes([0b0110_0111]))
        self.assertEqual((x, y), (0, 5))

    def test_basic_16_msb(self):
        (x, y) = bitalign_16_msb(array.array('H', [0]),
                                 array.array('H', [0]) )
        self.assertEqual((x, y), (0, 16))

        (x, y) = bitalign_16_msb(array.array('H', [0xffff, 0xffff]),
                                 array.array('H', [0x0000, 0x0000]))
        self.assertEqual((x, y), (-31, 0))

        (x, y) = bitalign_16_msb(array.array('H', [0xffff, 0x0000]),
                                 array.array('H', [0x0000, 0xffff]))
        self.assertEqual((x, y), (-16, 16))

    def reference_algorithm(self, A, B):
        N = len(A)
        self.assertEqual(len(B), N)
        self.assertNotEqual(N, 0)
        shift_by, common_bits = None, -1
        for a_start in reversed(range(N)):
            c = sum(map(operator.eq, A[a_start:], B))
            if c > common_bits:
                shift_by, common_bits = -a_start, c
        for b_start in range(1, N):
            c = sum(map(operator.eq, A, B[b_start:]))
            if c > common_bits:
                shift_by, common_bits = b_start, c
        return (shift_by, common_bits)

    def test_reference_algorithm(self):
        # Same test cases as above.
        r = self.reference_algorithm
        self.assertEqual(r("1111111111111111", "1111111111111111"), (0, 16))
        self.assertEqual(r("1111111111111111", "0000000000000000"), (-15, 0))
        self.assertEqual(r("1111000000000000", "0000111100000000"), (4, 12))
        self.assertEqual(r("1001001000100101", "0110001001111100"), (-4, 10))
        self.assertEqual(r("11011110", "10110100"), (-1, 6))
        self.assertEqual(r("00000110", "01100111"), (0, 5))

        # From docstring
        self.assertEqual(r("0001001011111111", "1111100010010111"), (5, 11))
        self.assertEqual(r("1111100010010111", "0001001011111111"), (-5, 11))

    def string_to_array(self, s, word_bits, lsb_msb):
        self.assertIn(lsb_msb, {'lsb', 'msb'})
        self.assertEqual(len(s) % word_bits, 0)
        arr = array.array(ARRAY_BIT_TO_TYPECODE[word_bits])
        self.assertEqual(arr.itemsize, word_bits // 8)
        words = [s[i:i+word_bits] for i in range(0, len(s), word_bits)]
        if lsb_msb == 'lsb':
            for word in words:
                arr.append(int(word[::-1], 2))
        else:
            for word in words:
                arr.append(int(word, 2))
        return arr

    def randstring(self, n):
        self.assertTrue(n >= 0)
        r = random.getrandbits(n)
        return bin(r + (1 << n))[-n:]

    def check_8bit(self, string1, string2):
        ref = self.reference_algorithm(string1, string2)
        with self.subTest(string1=string1, string2=string2, o='lsb'):
            res1 = bitalign_8_lsb(self.string_to_array(string1, 8, 'lsb'),
                                  self.string_to_array(string2, 8, 'lsb'))
            self.assertEqual(res1, ref)
        with self.subTest(string1=string1, string2=string2, o='msb'):
            res2 = bitalign_8_msb(self.string_to_array(string1, 8, 'msb'),
                                  self.string_to_array(string2, 8, 'msb'))
            self.assertEqual(res2, ref)

    def check_16bit(self, string1, string2):
        ref = self.reference_algorithm(string1, string2)
        with self.subTest(string1=string1, string2=string2, o='lsb'):
            res1 = bitalign_16_lsb(self.string_to_array(string1, 16, 'lsb'),
                                   self.string_to_array(string2, 16, 'lsb'))
            self.assertEqual(res1, ref)
        with self.subTest(string1=string1, string2=string2, o='msb'):
            res2 = bitalign_16_msb(self.string_to_array(string1, 16, 'msb'),
                                   self.string_to_array(string2, 16, 'msb'))
            self.assertEqual(res2, ref)

    def check_32bit(self, string1, string2):
        ref = self.reference_algorithm(string1, string2)
        with self.subTest(string1=string1, string2=string2, o='lsb'):
            res1 = bitalign_32_lsb(self.string_to_array(string1, 32, 'lsb'),
                                   self.string_to_array(string2, 32, 'lsb'))
            self.assertEqual(res1, ref)
        with self.subTest(string1=string1, string2=string2, o='msb'):
            res2 = bitalign_32_msb(self.string_to_array(string1, 32, 'msb'),
                                   self.string_to_array(string2, 32, 'msb'))
            self.assertEqual(res2, ref)

    def check_64bit(self, string1, string2):
        ref = self.reference_algorithm(string1, string2)
        with self.subTest(string1=string1, string2=string2, o='lsb'):
            res1 = bitalign_64_lsb(self.string_to_array(string1, 64, 'lsb'),
                                   self.string_to_array(string2, 64, 'lsb'))
            self.assertEqual(res1, ref)
        with self.subTest(string1=string1, string2=string2, o='msb'):
            res2 = bitalign_64_msb(self.string_to_array(string1, 64, 'msb'),
                                   self.string_to_array(string2, 64, 'msb'))
            self.assertEqual(res2, ref)

    CASES = [
        "0"*192,
        "1"*192,
        "01"*(192//2),
        "011"*(192//3),
        ("0110100110010110100101100110100110010110011010010110100110010110" +
         "1001011001101001011010011001011001101001100101101001011001101001" +
         "1001011001101001011010011001011001101001100101101001011001101001"),
        *["0"*i + "1"*(192-i) for i in (2, 3, 5, 7)]
    ]

    def test_8bit(self):
        data = [[s[:k*8] for s in self.CASES] for k in (1, 2, 3)]
        for arr in data:
            for a in arr:
                for b in arr:
                    self.check_8bit(a, b)

    def test_8bit_random(self):
        for _ in range(10):
            for N in range(8, 11*8, 8):
                self.check_8bit(self.randstring(N), self.randstring(N))

    def test_16bit(self):
        data = [[s[:k*16] for s in self.CASES] for k in (1, 2, 3)]
        for arr in data:
            for a in arr:
                for b in arr:
                    self.check_16bit(a, b)

    def test_16bit_random(self):
        for _ in range(10):
            for N in range(16, 11*16, 16):
                self.check_16bit(self.randstring(N), self.randstring(N))

    def test_32bit(self):
        data = [[s[:k*32] for s in self.CASES] for k in (1, 2, 3)]
        for arr in data:
            for a in arr:
                for b in arr:
                    self.check_32bit(a, b)

    def test_32bit_random(self):
        for _ in range(10):
            for N in range(32, 11*32, 32):
                self.check_32bit(self.randstring(N), self.randstring(N))

    def test_64bit(self):
        data = [[s[:k*64] for s in self.CASES] for k in (1, 2, 3)]
        for arr in data:
            for a in arr:
                for b in arr:
                    self.check_64bit(a, b)

    def test_64bit_random(self):
        for _ in range(10):
            for N in range(64, 11*64, 64):
                self.check_64bit(self.randstring(N), self.randstring(N))

    def check_all(self, string1, string2):
        s2a = self.string_to_array
        res1 = bitalign_8_lsb(s2a(string1, 8, 'lsb'), s2a(string2, 8, 'lsb'))
        res2 = bitalign_16_lsb(s2a(string1, 16, 'lsb'), s2a(string2, 16, 'lsb'))
        res3 = bitalign_32_lsb(s2a(string1, 32, 'lsb'), s2a(string2, 32, 'lsb'))
        res4 = bitalign_64_lsb(s2a(string1, 64, 'lsb'), s2a(string2, 64, 'lsb'))
        res5 = bitalign_8_msb(s2a(string1, 8, 'msb'), s2a(string2, 8, 'msb'))
        res6 = bitalign_16_msb(s2a(string1, 16, 'msb'), s2a(string2, 16, 'msb'))
        res7 = bitalign_32_msb(s2a(string1, 32, 'msb'), s2a(string2, 32, 'msb'))
        res8 = bitalign_64_msb(s2a(string1, 64, 'msb'), s2a(string2, 64, 'msb'))
        # The pure-python algorithm is slow, so just check that the
        # different methods agree
        # ref = self.reference_algorithm(string1, string2)
        ref = res1
        msg = (string1, string2)
        self.assertEqual(res1, ref, msg=msg)
        self.assertEqual(res2, ref, msg=msg)
        self.assertEqual(res3, ref, msg=msg)
        self.assertEqual(res4, ref, msg=msg)
        self.assertEqual(res5, ref, msg=msg)
        self.assertEqual(res6, ref, msg=msg)
        self.assertEqual(res7, ref, msg=msg)
        self.assertEqual(res8, ref, msg=msg)

    def test_random_all(self):
        for _ in range(5000):
            for N in (64, 128, 192, 256):
                s1, s2 = self.randstring(N), self.randstring(N)
                self.check_all(s1, s2)


FUNCS_AND_ARGS = [
    (bitalign_8_lsb,    array.array('B', range(256))),
    (bitalign_16_lsb,   array.array('H', range(256))),
    (bitalign_32_lsb,   array.array('I', range(256))),
    (bitalign_64_lsb,   array.array('Q', range(256))),
    (bitalign_8_msb,    array.array('B', range(256))),
    (bitalign_16_msb,   array.array('H', range(256))),
    (bitalign_32_msb,   array.array('I', range(256))),
    (bitalign_64_msb,   array.array('Q', range(256))),
]


class BitAlignAPITest(unittest.TestCase):

    def test_bad_arguments(self):
        for f, a in FUNCS_AND_ARGS:
            with self.assertRaisesRegex(TypeError, "expected 2 arguments"):
                f()
            with self.assertRaisesRegex(TypeError, "expected 2 arguments"):
                f(a)
            with self.assertRaisesRegex(TypeError, "expected 2 arguments"):
                f(a, a, a)
            with self.assertRaisesRegex(TypeError, "arguments"):
                f(a=a, b=a)

    @unittest.skipIf(platform.python_implementation() == "PyPy", "fails on pypy")
    def test_not_contiguous(self):
        for f, a in FUNCS_AND_ARGS:
            m = memoryview(a)
            f(m[1:], m[:-1])
            with self.assertRaisesRegex(BufferError, "not C-contiguous"):
                f(m, m[1::2])
            with self.assertRaisesRegex(BufferError, "not C-contiguous"):
                f(m, m[1::2])

    def test_different_lengths(self):
        for f, a in FUNCS_AND_ARGS:
            with self.assertRaisesRegex(ValueError, "same length"):
                f(a[:10], a[:9])

    def test_bad_itemsize(self):
        for f1, a1 in FUNCS_AND_ARGS:
            for f2, a2 in FUNCS_AND_ARGS:
                if a1.itemsize != a2.itemsize:
                    with self.assertRaisesRegex(ValueError, "incorrect itemsize"):
                        f1(a2, a2)

    def test_empty(self):
        for f, a in FUNCS_AND_ARGS:
            with self.assertRaisesRegex(ValueError, "cannot be empty"):
                f(a[0:0], a[0:0])

    def test_too_large(self):
        for f, a in FUNCS_AND_ARGS:
            big = a[0:1] * ((2**31-1) // (8 * 2 * a.itemsize))
            with self.assertRaisesRegex(ValueError, "Buffers are too large"):
                f(big, big)

    def test_have_docstrings(self):
        for f, a in FUNCS_AND_ARGS:
            assert len(f.__doc__) >= 200


if __name__ == "__main__":
    unittest.main()

