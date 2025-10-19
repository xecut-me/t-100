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

errors = 0
received = 0

while True:
    t1=sys.stdin.buffer.read(1)
    received = received + 1
    i = t1[0]
    t2=next(generator)
    if i!=t2:
        errors = errors + 1
        diff = i ^ t2
        binary_string = format(diff, '08b')
        t2_bin = format(t2,'08b')
        i_bin = format(i,'08b')
        error_rate = 100.0* errors/received
        print(f"ERROR: wrong bits: {binary_string} expected: {t2_bin} got {i_bin} error rate: {error_rate}")
    else:
        error_rate = 100.0* errors/received
        print(f". error rate: {error_rate}")
