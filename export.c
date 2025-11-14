// gcc -std=c11 -O2 -Wall -Wextra export.c -o export.elf
// ./export.elf

#define PROGMEM

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "arduino/convert.h"

enum
{
    ASCII2BAUDOT_LEN = 128,
    BAUDOT2ASCII_LEN = 64
};

static void fprint_uint8_array_json(FILE *out, const char *name,
                                    const uint8_t *arr, size_t n)
{
    fprintf(out, "  \"%s\": [", name);
    for (size_t i = 0; i < n; ++i)
    {
        if (i % 16 == 0)
        {
            fprintf(out, "\n    ");
        }
        fprintf(out, "%u", (unsigned)arr[i]);
        if (i + 1 < n)
        {
            fputs(", ", out);
        }
    }
    fputs("\n  ]", out);
}

void export_tables()
{
    const char *out_path = "convert.json";

    FILE *out = fopen(out_path, "w");
    if (!out)
    {
        perror("fopen");
        return;
    }

    fputs("{\n", out);
    fprint_uint8_array_json(out, "ascii2baudot", ascii2baudot, ASCII2BAUDOT_LEN);
    fputs(",\n", out);
    fprint_uint8_array_json(out, "baudot2ascii", baudot2ascii, BAUDOT2ASCII_LEN);
    fputs("\n}\n", out);

    if (fclose(out) != 0)
    {
        perror("fclose");
        return;
    }
}

static int is_printable(int c) { return c >= 32 && c <= 126; }

static void print_escaped_char(unsigned char c)
{
    switch (c)
    {
    case '\n':
        fputs("\\n", stdout);
        break;
    case '\r':
        fputs("\\r", stdout);
        break;
    case '\t':
        fputs("\\t", stdout);
        break;
    case '\\':
        fputs("\\\\", stdout);
        break;
    case '\"':
        fputs("\\\"", stdout);
        break;
    default:
        if (is_printable(c))
            putchar(c);
        else
            printf("\\x%02X", c);
    }
}

void print_charset()
{
    fputs("\"", stdout);

    for (int ascii = 0; ascii < ASCII2BAUDOT_LEN; ++ascii)
    {
        uint8_t baudot = ascii2baudot[ascii];
        uint8_t idx = baudot % 32;

        if (ascii == baudot2ascii[idx] || ascii == baudot2ascii[32 + idx])
        {
            print_escaped_char((unsigned char)ascii);
        }
    }

    fputs("\"", stdout);
    fputs("\n", stdout);
}

int main()
{
    export_tables();
    print_charset();
    return 0;
}
