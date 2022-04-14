#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
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
    DDRD &= ~_BV(PD2);
    PORTD |= _BV(PD2);

    DDRC &= ~(_BV(3) | _BV(4) | _BV(5));
    PORTC |= _BV(3) | _BV(4) | _BV(5);
}

static void DoGoSleep()
{
    GICR  |= _BV(INT0);                  // enable INT0;
    MCUCR &= ~(_BV(ISC01) | _BV(ISC00));

    DDRD &= ~_BV(PD2);
    PORTD |= _BV(PD2);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    SetNumberShow(false);
    OnPrinterTick(0);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sei();
    sleep_cpu();
    sleep_disable();
    SetNumberShow(true);

    GICR  &= ~_BV(INT0);                  // disable INT0;
    cli();
    PrepareTimer();
    PrepareButtons();
}

static uint8_t g_ticks = 0;
static bool g_Sleep = false;
void GoSleep()
{
    g_Sleep = true;
}

void MainLoop()
{
    uint8_t cnt = 0;
    for (;;)
    {
        if (g_Sleep)
        {
            DoGoSleep();
            g_Sleep = false;
        }
        bool tick = false;
        ATOMIC_BLOCK(ATOMIC_FORCEON)
        {
            if (g_ticks > 0)
            {
                tick = true;
                -- g_ticks;
            }
        }
        OnGpio();
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
{}
