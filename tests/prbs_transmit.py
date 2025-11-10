#!/usr/bin/env python3
import time
import sys
import os


def prbs15_gen(seed=1):
    """
    Generator for PRBS15 sequence using a 15-bit LFSR.
    :param seed: Initial state (non-zero 15-bit integer)
    :yield: Next bit in the PRBS sequence (0 or 1)
    """
    lfsr = seed & 0x7FFF  # 15-bit register mask
    while True:
        new_bit = ((lfsr >> 14) ^ (lfsr >> 13)) & 1  # XOR of bits 14 and 13
        bits = lfsr & 0x1f  # output least significant bit
        lfsr = ((lfsr << 1) | new_bit) & 0x7FFF  # shift and insert new bit
        yield bits


generator = prbs15_gen(seed=0x3)

with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
    while True:
        i = next(generator)
        stdout.write(i.to_bytes(1))
        stdout.flush()
        time.sleep(0.2)
