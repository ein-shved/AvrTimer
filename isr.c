#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

void PrepareTimer()
{
    TCCR1B |= (1 << WGM12);
    TIMSK |= (1 << OCIE1A);
    sei();
    OCR1A   = (F_CPU/64)/SECOND_DIVIDER;

    TCCR1B |= ((1 << CS10) | (1 << CS11));
}

void PrepareButtons()
{
    GICR  |= _BV(INT0);                  // enable INT0;
//    MCUCR &= ~_BV(ISC01);
//    MCUCR |= _BV(ISC00);
//
    MCUCR &= ~(_BV(ISC01) | _BV(ISC00));
    sei();              // enable global interrupt



    DDRD &= ~_BV(PD2);
    PORTD |= _BV(PD2);

    DDRC &= ~(_BV(3) | _BV(4) | _BV(5));
    PORTC |= _BV(3) | _BV(4) | _BV(5);
}

static uint8_t g_ticks = 0;
static uint8_t g_gpios = 0;

void MainLoop()
{
    uint8_t cnt = 0;
    for (;;)
    {
        bool tick = false, gpio = true;
        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
            if (g_ticks > 0)
            {
                tick = true;
                -- g_ticks;
            }
            if (g_gpios > 0)
            {
                gpio = true;
                -- g_gpios;
            }
        }
        if (gpio)
        {
            OnGpio();
        }
        if (tick)
        {
            OnFsmTick();
            OnGpioTick();
        }
        OnPrinterTick(++cnt);
    }
}

ISR(TIMER1_COMPA_vect)
{
    ++g_ticks;
}

ISR(INT0_vect)
{
    ++g_gpios;
}
