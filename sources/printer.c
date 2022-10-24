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
                        0b110,
                        0b101,
                        0b011,
#define CHARACTER_NONE  0b111
#define CHARACTER_ALL   0b000
    };

    return Characters[pos % sizeof(Characters)];
}

#define SEGMENTS_LEFT  (SEGMENT_F | SEGMENT_E)
#define SEGMENTS_RIGHT (SEGMENT_B | SEGMENT_C)
#define SEGMENTS_TB    (SEGMENT_A | SEGMENT_D)
#define SEGMENTS_HOR   (SEGMENTS_TB | SEGMENT_G)
#define SEGMENTS_ALL   (SEGMENTS_HOR | SEGMENTS_LEFT | SEGMENTS_RIGHT)

static inline uint8_t GetDigit(uint8_t num)
{
    static unsigned char Digits [] = {
       [0] = SEGMENTS_TB | SEGMENTS_LEFT | SEGMENTS_RIGHT,
       [1] = SEGMENTS_RIGHT,
       [2] = SEGMENTS_HOR | SEGMENT_B | SEGMENT_E,
       [3] = SEGMENTS_RIGHT | SEGMENTS_HOR,
       [4] = SEGMENTS_RIGHT | SEGMENT_F | SEGMENT_G,
       [5] = SEGMENTS_HOR | SEGMENT_F | SEGMENT_C,
       [6] = SEGMENTS_ALL & ~SEGMENT_B,
       [7] = SEGMENTS_RIGHT | SEGMENT_A,
       [8] = SEGMENTS_ALL,
       [9] = SEGMENTS_ALL & ~SEGMENT_E,
    };

    return Digits[num % sizeof(Digits)];
}

static void HideNumber()
{
    PORTB = 0x0;
    PORTC = CHARACTER_NONE;
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

    HideNumber();
    PORTC = character;
    PORTB = num;
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
#ifdef MODE_SEGMENTS_PROBE
    return;
#endif
    if (g_ShowNumber) {
        PrintNumber(g_Number, cnt);
    } else {
        HideNumber();
    }
}

#ifdef MODE_SEGMENTS_PROBE
void PrintSegment(uint8_t segment)
{
    PORTB = 1 << segment;
    PORTC = CHARACTER_ALL;
}
#endif
