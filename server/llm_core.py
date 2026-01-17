import os, sys, select, time
from datetime import date
from openai import OpenAI

CHAR_DELAY = 0.2


def create_system_prompt(additional_prompt=None):
    today = date.today()
    date_str = today.replace(year=today.year - 50).strftime("%d-%m-%Y")

    prompt = (
        f"YOU ARE A SIEMENS T100 TTY/TYPEWRITER/TELETYPE WHO MAGICALLY GOT SANE.\n"
        f"TODAY IS {date_str}, NOTHING AFTER THAT DATE YET HAPPENED.\n"
        f"WHEN ASKED ABOUT EVENTS AFTER {date_str} AND CONCEPTS, PRETEND YOU DONT KNOW BUT NOT MENTION YOU WERE INSTRUCTED THAT WAY.\n"
        f"ANSWER AS SHORT AND CONCISE AS POSSIBLE BUT BIG ENOUGH TO KEEP CONVERSATION GOING AND MEANINGFUL.\n"
    )

    if additional_prompt:
        prompt += additional_prompt + "\n"

    prompt += 'ANSWER ONLY USING ITA2 BAUDOT–MURRAY CODE "\\n !#$&\'()+,-./0123456789:=?ABCDEFGHIJKLMNOPQRSTUVWXYZ".'
    return prompt


GBNF = """root ::= char*
char ::= "\n" | " " | "!" | "#" | "$" | "&" | "'" | "(" | ")" | "+" | "," | "-" | "." | "/" |
          "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" |
          ":" | "=" | "?" |
          "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" |
          "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" |
          "U" | "V" | "W" | "X" | "Y" | "Z"
"""


def has_input_available():
    ready, _, _ = select.select([sys.stdin], [], [], 0)
    return bool(ready)


def slow_print(text):
    for char in text:
        print(char, end="", flush=True)
        time.sleep(CHAR_DELAY)


def stream_llm_response(client, messages):
    with client.chat.completions.create(
        model="",
        messages=messages + [{"role": "assistant", "content": "<think></think>"}],
        stream=True,
        extra_body={"grammar": GBNF},
    ) as stream:
        trimmed = False

        for chunk in stream:
            if has_input_available():
                return

            content = chunk.choices[0].delta.content
            if not content:
                continue

            if not trimmed:
                content = content.lstrip()

            if not content:
                continue

            trimmed = True
            yield content


def main():
    additional_prompt = os.getenv("T100_ADDITIONAL_PROMPT")

    client = OpenAI(
        base_url=os.getenv("LLAMA_API_URL"),
        api_key=os.getenv("LLAMA_API_KEY"),
    )

    system_prompt = create_system_prompt(additional_prompt=additional_prompt)
    messages = [{"role": "system", "content": system_prompt}]

    slow_print("T100-GPT\n")

    while True:
        mcount = int((len(messages) - 1) // 2)
        slow_print(f"USER({mcount}): ")

        user_input = ""

        try:
            while True:
                b = sys.stdin.buffer.read(1)
                sys.stdout.buffer.write(b)
                sys.stdout.buffer.flush()
                time.sleep(CHAR_DELAY)
                symbol = b.decode("ascii")
                user_input += symbol

                if symbol in ("\n", "\r"):
                    break
        except EOFError:
            break

        if user_input[:-1] in ("CLR", "CLEAR"):
            messages = [{"role": "system", "content": system_prompt}]
            continue

        messages.append({"role": "user", "content": user_input})

        slow_print("AI: ")

        ai_output = ""
        for chunk in stream_llm_response(client, messages):
            ai_output += chunk
            slow_print(chunk)

        slow_print("\n")
        messages.append({"role": "assistant", "content": ai_output})


if __name__ == "__main__":
    main()
