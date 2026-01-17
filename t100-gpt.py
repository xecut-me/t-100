import argparse, requests, time, json, os, socket
from datetime import date

TCP_HOST = "192.168.10.124"
TCP_PORT = 1337


def output(string, sock, WAIT=0.25):
    for char in string:
        sock.sendall(char.encode("ascii"))
        time.sleep(WAIT)


parser = argparse.ArgumentParser()
parser.add_argument("--german", action="store_true", help="Enable German mode")
parser.add_argument("--russian", action="store_true", help="Enable Russian mode")
args = parser.parse_args()

today = date.today()
DATE_STR = today.replace(year=today.year - 50).strftime("%d-%m-%Y")

system_prompt = (
    f"YOU ARE A SIEMENS T100 TTY/TYPEWRITER/TELETYPE WHO MAGICALLY GOT SANE.\n"
)
system_prompt += f"TODAY IS {DATE_STR}, NOTHING AFTER THAT DATE YET HAPPENED.\n"
system_prompt += f"WHEN ASKED ABOUT EVENTS AFTER {DATE_STR} AND CONCEPTS, PRETEND YOU DONT KNOW BUT NOT MENTION YOU WERE INSTRUCTED THAT WAY.\n"
system_prompt += f"ANSWER AS SHORT AND CONCISE AS POSSIBLE BUT BIG ENOUGH TO KEEP CONVERSATION GOING AND MEANINGFUL.\n"

if args.russian:
    system_prompt += f"IMPORTANT: WRITE ONLY IN RUSSIAN BUT IN LATIN CHARACTERS.\n"

if args.german:
    system_prompt += f"ADD SOME GERMAN LOOKING WORDS FROM TIME TO TIME.\n"

system_prompt += f'ANSWER ONLY USING ITA2 BAUDOT–MURRAY CODE "\\n !#$&\'()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ".'


messages = [{"role": "system", "content": system_prompt}]

GBNF = """root ::= char*
char ::= "\n" | " " | "!" | "#" | "$" | "&" | "'" | "(" | ")" | "+" | "," | "-" | "." | "/" |
          "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
          ":" | "=" | "?" |
          "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
          "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
          "U" | "V" | "W" | "X" | "Y" | "Z"
"""

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_HOST, TCP_PORT))

try:
    output("T100-GPT\n", sock)

    while True:
        output("USER: ", sock)

        buf = ""

        while True:
            ch = sock.recv(1).decode("ascii")
            if not ch:
                raise ConnectionError("Connection closed")
            output(ch, sock)
            if ch == "\n":
                break
            buf += ch

        messages += [{"role": "user", "content": buf}]

        output("AI: ", sock)

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
                        output(chunk, sock)
                    except Exception as e:
                        pass

        messages += [{"role": "assistant", "content": ai_output}]
        output("\n", sock)
except KeyboardInterrupt:
    pass
finally:
    sock.close()
