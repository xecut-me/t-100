import argparse, requests, termios, serial, time, json, sys, os
from datetime import date

parser = argparse.ArgumentParser()
parser.add_argument(
    "--device", default=None, help="Serial device path, e.g. /dev/ttyUSB0"
)
parser.add_argument("--german", action="store_true", help="Enable German mode")
args = parser.parse_args()

USE_SERIAL = args.device is not None

fd = None
orig_attrs = None

if USE_SERIAL:
    in_stream = out_stream = serial.Serial(
        port=args.device,
        baudrate=9600,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=None,
    )
else:
    in_stream = sys.stdin
    out_stream = sys.stdout

    fd = sys.stdin.fileno()
    orig_attrs = termios.tcgetattr(fd)
    attrs = termios.tcgetattr(fd)

    attrs[3] &= ~(termios.ECHO | termios.ICANON)
    termios.tcsetattr(fd, termios.TCSANOW, attrs)


def output(data, WAIT=0.25):
    if isinstance(data, str):
        text = data
        raw = data.encode("ascii", errors="replace")
    else:
        raw = bytes(data)
        text = raw.decode("ascii", errors="replace")

    if USE_SERIAL:
        for b in raw:
            out_stream.write(bytes([b]))
            out_stream.flush()
            time.sleep(WAIT)
    else:
        for ch in text:
            out_stream.write(ch)
            out_stream.flush()
            time.sleep(WAIT)


today = date.today()
DATE_STR = today.replace(year=today.year - 50).strftime("%d-%m-%Y")

messages = [
    {
        "role": "system",
        "content": f"""YOU ARE A SIEMENS T100 TTY/TYPEWRITER/TELETYPE WHO MAGICALLY GOT SANE.
TODAY IS {DATE_STR}, NOTHING AFTER THAT DATE YET HAPPENED.
WHEN ASKED ABOUT EVENTS AFTER {DATE_STR} AND CONCEPTS, PRETEND YOU DONT KNOW BUT NOT MENTION YOU WERE INSTRUCTED THAT WAY.
ANSWER AS SHORT AND CONCISE AS POSSIBLE BUT BIG ENOUGH TO KEEP CONVERSATION GOING AND MEANINGFUL.
ADD SOME GERMAN LOOKING WORDS FROM TIME TO TIME.
ANSWER ONLY USING ITA2 BAUDOT–MURRAY CODE "\\n !#$&'()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ".""",
    }
]

GBNF = """root ::= char*
char ::= "\n" | " " | "!" | "#" | "$" | "&" | "'" | "(" | ")" | "+" | "," | "-" | "." | "/" |
          "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
          ":" | "=" | "?" |
          "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
          "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
          "U" | "V" | "W" | "X" | "Y" | "Z"
"""

try:
    output("T100-GPT\n")

    while True:
        output("USER: ")
        in_buffer = ""

        while True:
            ch = in_stream.read(1)
            if not ch:
                continue

            if USE_SERIAL:

                if ch in (b"\r", b"\n"):

                    output("\r\n")
                    break
                char = ch.decode("ascii", errors="replace")
                output(char)
            else:

                char = ch
                output(char)
                if char == "\n":
                    break

            in_buffer += char

        messages.append({"role": "user", "content": in_buffer})

        output("AI: ")
        out_buffer = ""

        with requests.post(
            os.getenv("LLAMA_API_URL"),
            headers={
                "Content-Type": "application/json",
                "Authorization": "Bearer " + os.getenv("LLAMA_API_KEY", ""),
            },
            data=json.dumps(
                {
                    "messages": messages
                    + [{"role": "assistant", "content": "<think></think>"}],
                    "stream": True,
                    "grammar": GBNF,
                }
            ),
            stream=True,
        ) as response:
            response.raise_for_status()
            trimmed = False

            for line in response.iter_lines(decode_unicode=True):
                if not line:
                    continue
                if not line.startswith("data: "):
                    continue

                try:
                    payload = json.loads(line[6:])
                    delta = payload["choices"][0]["delta"].get("content", "")
                except Exception:
                    continue

                chunk = delta
                if not trimmed:
                    chunk = chunk.lstrip()

                if not chunk:
                    continue

                trimmed = True
                out_buffer += chunk
                output(chunk)

        messages.append({"role": "assistant", "content": out_buffer})
        output("\n")

except KeyboardInterrupt:
    pass
finally:
    if USE_SERIAL:
        try:
            in_stream.close()
        except Exception:
            pass
    else:
        if fd is not None and orig_attrs is not None:
            try:
                termios.tcsetattr(fd, termios.TCSANOW, orig_attrs)
            except Exception:
                pass
