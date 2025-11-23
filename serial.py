import serial

parser.add_argument(
    "--device", default=None, help="Serial device path, e.g. /dev/ttyUSB0"
)

USE_SERIAL = args.device is not None

in_stream = out_stream = serial.Serial(
    port=args.device,
    baudrate=9600,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=None,
)

sys.stdin.buffer

in_stream.close()
