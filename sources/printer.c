#include "timer.h"

#include <avr/io.h>

#ifndef MAX_NUMBER
#define MAX_NUMBER 1000
#endif /* MAX_NUMBER */

static uint16_t g_Number = 0;
static bool g_ShowNumber = true;

void PutNumber(uint16_t num)
{
    g_Number = num % MAX_NUMBER;
}

uint16_t GetNumber()
{
    return g_Number;
}

void SetNumberShow(bool show)
{
    g_ShowNumber = show;
}

/* Return mask for specific positin in number */
static inline uint8_t GetCharacter(uint8_t pos)
{
    static uint8_t Characters [] = {
                        0b011,
                        0b101,
                        0b110,
#define CHARACTER_NONE  0b111
    };

    return Characters[pos % sizeof(Characters)];
}

static inline uint8_t GetDigit(uint8_t num)
{
    static unsigned char Digits [] = {
        0b11111010,
        0b00100010,
        0b10111001,
        0b10101011,
        0b01100011,
        0b11001011,
        0b11011011,
        0b10100010,
        0b11111011,
        0b11100011,
    };

    return Digits[num % sizeof(Digits)];
}

static void PrintNumber(uint16_t num, uint8_t cnt)
{
    cnt = cnt % 3;

    uint8_t character = GetCharacter(cnt);

    while (cnt > 0)
    {
        num /= 10;
        --cnt;
    }
    num = GetDigit(num % 10);

    PORTC = CHARACTER_NONE;
    PORTB = num;
    PORTC = character;
}

static void HideNumber()
{
    PORTC = CHARACTER_NONE;
}

void PreparePrinter()
{
    DDRB = 0xff;
    DDRC |= 0b111;
    PORTB = 0x0;
    PORTC = CHARACTER_NONE;
}

void OnPrinterTick(uint8_t cnt)
{
    if (g_ShowNumber) {
        PrintNumber(g_Number, cnt);
    } else {
        HideNumber();
    }
}
