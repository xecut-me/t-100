import argparse, requests, termios, time, json, sys, os
from datetime import date


def output(string, WAIT=0.25):
    for char in string:
        sys.stdout.write(char)
        sys.stdout.flush()
        time.sleep(WAIT)


parser = argparse.ArgumentParser()
parser.add_argument("--german", action="store_true", help="Enable German mode")
args = parser.parse_args()

fd = sys.stdin.fileno()
orig_attrs = termios.tcgetattr(fd)
attrs = termios.tcgetattr(fd)

today = date.today()
DATE_STR = today.replace(year=today.year - 50).strftime("%d-%m-%Y")

system_prompt = (
    f"YOU ARE A SIEMENS T100 TTY/TYPEWRITER/TELETYPE WHO MAGICALLY GOT SANE.\n"
)
system_prompt += f"TODAY IS {DATE_STR}, NOTHING AFTER THAT DATE YET HAPPENED.\n"
system_prompt += f"WHEN ASKED ABOUT EVENTS AFTER {DATE_STR} AND CONCEPTS, PRETEND YOU DONT KNOW BUT NOT MENTION YOU WERE INSTRUCTED THAT WAY.\n"
system_prompt += f"ANSWER AS SHORT AND CONCISE AS POSSIBLE BUT BIG ENOUGH TO KEEP CONVERSATION GOING AND MEANINGFUL.\n"

if args.german:
    system_prompt += f"ADD SOME GERMAN LOOKING WORDS FROM TIME TO TIME.\n"

system_prompt += f'ANSWER ONLY USING ITA2 BAUDOT–MURRAY CODE "\\n !#$&\'()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ".'

sys.stderr.write(f"System prompt:\n{system_prompt}\n")

messages = [{"role": "system", "content": system_prompt}]

GBNF = """root ::= char*
char ::= "\n" | " " | "!" | "#" | "$" | "&" | "'" | "(" | ")" | "+" | "," | "-" | "." | "/" |
          "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
          ":" | "=" | "?" |
          "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
          "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
          "U" | "V" | "W" | "X" | "Y" | "Z"
"""

try:
    attrs[3] &= ~(termios.ECHO | termios.ICANON)
    termios.tcsetattr(fd, termios.TCSANOW, attrs)

    output("T100-GPT\n")

    while True:
        output("USER: ")

        buf = ""

        while True:
            ch = sys.stdin.read(1)
            output(ch)
            if ch == "\n":
                break
            buf += ch

        messages += [{"role": "user", "content": buf}]

        output("AI: ")

        ai_output = ""

        with requests.post(
            os.getenv("LLAMA_API_URL"),
            headers={
                "Content-Type": "application/json",
                "Authorization": "Bearer " + os.getenv("LLAMA_API_KEY"),
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
                if line.startswith("data: "):
                    try:
                        chunk = json.loads(line[6:])["choices"][0]["delta"]["content"]

                        if not trimmed:
                            chunk = chunk.lstrip()

                        if not chunk:
                            continue

                        trimmed = True
                        ai_output += chunk
                        output(chunk)
                    except Exception as e:
                        pass

        messages += [{"role": "assistant", "content": ai_output}]
        output("\n")
except KeyboardInterrupt:
    pass

termios.tcsetattr(fd, termios.TCSANOW, orig_attrs)
