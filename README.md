# BitAlign

*Given two strings of bits, find how to best align them so that they have the most bits in common.*

## Installation: `pip install bitalign`

## Examples

The following snippets show a conceptual string of 16 bits `A = "0001001011111111"`
needs to shifted to the right by 5 in order to best line up with
the string of 16 bits `B = "1111100010010111"`, at
which point they have 11 bits in common:

```
A =      0001001011111111
B = 1111100010010111
```

The only differences are how we decide to encode the bits into a list of integer.

```pycon
>>> from bitalign import *

>>> # We can can treat bytes objects like b'\x12\xff' as bit arrays
>>> A = bytes([0b0001_0010, 0b1111_1111])
>>> B = bytes([0b1111_1000, 0b1001_0111])
>>> bitalign_8_msb(A, B)
(5, 11)

>>> # Reversing the bits in each byte --> use "lsb" method instead
>>> A = bytes([0b0100_1000, 0b1111_1111])
>>> B = bytes([0b0001_1111, 0b1110_1001])
>>> bitalign_8_lsb(A, B)
(5, 11)

>>> # numpy arrays also work, just make sure they have an appropriate dtype.
>>> import numpy as np
>>> A = np.array([0b0001001011111111], dtype=np.uint16)
>>> B = np.array([0b1111100010010111], dtype=np.uint16)
>>> bitalign_16_msb(A, B)
(5, 11)

>>> # Reverse bits in each uint64 --> use "lsb" method instead
>>> A = np.array([0b1111111101001000], dtype=np.uint16)
>>> B = np.array([0b1110100100011111], dtype=np.uint16)
>>> bitalign_16_lsb(A, B)
(5, 11)

>>> # Works on general pairs of c-contiguous buffer objects
>>> # numpy.array, array.array, bytes, bytearray, memoryview, etc.
>>> import array
>>> A = array.array('H', [0b0001001011111111])
>>> B = array.array('H', [0b1111100010010111])
>>> bitalign_16_msb(A, B)
(5, 11)
```

## API

This `bitalign` package exposes 8 methods:

```python
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
```

```
    bitalign_#_???(arr1, arr2) --> (shift_by, num_common_bits);

    Return a tuple (x, y) such that when arr1 is shifted by x bits,
    the number of bits in common between arr1 and arr2 is y.

    Positive shifts indicate that arr1 needs to be shifted toward the back:

        arr1 =                    -->  0001001011111111  -->
        arr2 =                    1111100010010111
        gives (shift_by=5, num_common_bits=11)

    Negative shifts indicate that arr1 needs to be shifted toward the front:

        arr1 =          <--  1111100010010111  <--
        arr2 =                    0001001011111111
        gives (shift_by=-5, num_common_bits=11)

    The number (8, 16, 32, or 64) in the function name the number of bits
    that must be in each array entry.  'lsb'/'msb' indicates whether the
    0th bit of each logical bit-array is to be stored in the least or most
    significant bit of arr[0].

    If more than one shift is optimal, the negative-most shift is used.
```
