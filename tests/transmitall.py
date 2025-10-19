#!/usr/bin/env python3

# This code transmits all possible code combinations

import time
import sys
import os

while True:
    with os.fdopen(sys.stdout.fileno(), "wb", closefd=False) as stdout:
        for i in range(0,32):
            stdout.write(i.to_bytes(1))
            stdout.flush()    
            time.sleep(0.2)
