#!/usr/bin/env python3

import time
import sys
import os

while True:
    t=sys.stdin.buffer.read(1)
    binary_string = ''.join(format(byte, '08b') for byte in t)
    print(binary_string)
