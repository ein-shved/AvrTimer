#include "timer.h"

#include <avr/io.h>
#include <avr/eeprom.h>

static enum State g_State = STATE_IDLE;

typedef void (*OnTick_f)(void);
typedef void (*OnEnter_f)(void);
typedef void (*OnButton_f)(enum Button btn, enum ButtonAction action);

static void Idle_OnTick();
static void Idle_OnButton(enum Button btn, enum ButtonAction action);
static void Idle_OnEnter();

static void Ticking_OnTick();
static void Ticking_OnEnter();
static void Ticking_OnButton(enum Button btn, enum ButtonAction action);

static void Finished_OnTick();
static void Finished_OnEnter();
static void Finished_OnButton(enum Button btn, enum ButtonAction action);

static void Edit_OnEnter();
static void Edit_OnTick();
static void Edit_OnButton(enum Button btn, enum ButtonAction action);

#ifdef MODE_SEGMENTS_PROBE
static void ProbeSegment_OnEnter();
static void ProbeSegment_OnTick();
static void ProbeSegment_OnButton(enum Button btn, enum ButtonAction action);
#endif

static struct Fsm {
    OnTick_f onTick;
    OnButton_f onButton;
    OnEnter_f onEnter;
} g_Fsm[] = {
    [STATE_IDLE] = {
        .onTick = Idle_OnTick,
        .onButton = Idle_OnButton,
        .onEnter = Idle_OnEnter,
    },
    [STATE_TICKING] = {
        .onTick = Ticking_OnTick,
        .onButton = Ticking_OnButton,
        .onEnter = Ticking_OnEnter,
    },
    [STATE_FINISHED] = {
        .onTick = Finished_OnTick,
        .onButton = Finished_OnButton,
        .onEnter = Finished_OnEnter,
    },
    [STATE_EDIT_PLUS] = {
        .onTick = Edit_OnTick,
        .onButton = Edit_OnButton,
        .onEnter = Edit_OnEnter,
    },
    [STATE_EDIT_MINUS] = {
        .onTick = Edit_OnTick,
        .onButton = Edit_OnButton,
        .onEnter = Edit_OnEnter,
    },
#ifdef MODE_SEGMENTS_PROBE
    [STATE_SEGMENTS_PROBE] = {
        .onTick = ProbeSegment_OnTick,
        .onButton = ProbeSegment_OnButton,
        .onEnter = ProbeSegment_OnEnter,
    },
#endif
};

static uint16_t g_Timeout = DEFAULT_TIMEOUT;
static uint8_t g_Ticking_counter = 0;

void OnFsmTick(void)
{
    g_Fsm[g_State % STATE_NUM].onTick();
}

void OnButton(enum Button btn, enum ButtonAction action)
{
    g_Fsm[g_State % STATE_NUM].onButton(btn, action);
}

void ToState(enum State new_state)
{
    g_State = new_state;
    g_Fsm[g_State % STATE_NUM].onEnter();
}

static void SaveEeprom()
{
    eeprom_busy_wait();
    eeprom_write_word(EEPROM_TIMER_POS, g_Timeout);
}

void PrepareFsm()
{
    DDRD |= ALARM_PIN;
    PORTD &= ~ALARM_PIN;

    eeprom_busy_wait();
    g_Timeout = eeprom_read_word(EEPROM_TIMER_POS);
    if (g_Timeout < 1 || g_Timeout > 999)
    {
        g_Timeout = DEFAULT_TIMEOUT;
        SaveEeprom();
    }
#ifdef MODE_SEGMENTS_PROBE
    ToState(STATE_SEGMENTS_PROBE);
#else
    ToState(STATE_IDLE);
#endif

}

void SetAlarm(bool on)
{
    if (on)
    {
        PORTD |= ALARM_PIN;
    }
    else
    {
        PORTD &= ~ALARM_PIN;
    }
}

static void DoEdit(int Num)
{
    if ((Num < 0 && g_Timeout > -Num) || (Num > 0 && g_Timeout < 999 -Num))
    {
        g_Timeout += Num;
        PutNumber(g_Timeout);
    }
}

static void Idle_OnEnter()
{
    SetNumberShow(true);
    SetAlarm(false);
    PutNumber(g_Timeout);
    g_Ticking_counter = 0;
}

static void Idle_OnTick()
{
#if SLEEP_TIMEOUT_TICKS > 0
    if(++g_Ticking_counter >= SLEEP_TIMEOUT_TICKS)
    {
        g_Ticking_counter = 0;
        GoSleep();
    }
#endif
}

static void Idle_OnButton(enum Button btn, enum ButtonAction action)
{
    if (btn == BTN_RUN && (action == BTN_CLICKED || action == BTN_HOLD))
    {
        ToState(STATE_TICKING);
        return;
    }
    if (action == BTN_CLICKED || action == BTN_HOLD)
    {
        switch (btn)
        {
        case BTN_PLUS:
            if (action == BTN_CLICKED)
            {
                DoEdit(1);
                SaveEeprom();
            }
            else
            {
                ToState(STATE_EDIT_PLUS);
            }
            break;
        case BTN_MINUS:
            if (action == BTN_CLICKED)
            {
                DoEdit(-1);
                SaveEeprom();
            }
            else
            {
                ToState(STATE_EDIT_MINUS);
            }
            break;
        default:
            break;
        }
    }
}

static void Ticking_OnEnter()
{
    g_Ticking_counter = 0;
}

static void Ticking_OnTick()
{
    uint16_t num = GetNumber();
    g_Ticking_counter = (g_Ticking_counter + 1) % SECOND_DIVIDER;
    if (!g_Ticking_counter) {
        if (num == 0) {
            ToState(STATE_FINISHED);
            return;
        } else {
            num-=1;
        }
        PutNumber(num);
    }

    if (num > NOTIFY_THREASHOLD) {
        SetAlarm(false);
        SetNumberShow(true);
    } else {
        if(g_Ticking_counter > (SECOND_DIVIDER / 2)) {
            SetNumberShow(false);
            SetAlarm(false);
        } else {
            SetNumberShow(true);
            SetAlarm(true);
        }
    }
}

static void Ticking_OnButton(enum Button btn, enum ButtonAction action)
{
    if (btn == BTN_RUN && (action == BTN_CLICKED || action == BTN_HOLD))
    {
        ToState(STATE_IDLE);
    }
}

static uint8_t g_Seconds_counter = 0;
static void Finished_OnEnter()
{
    g_Seconds_counter = 0;
    SetNumberShow(true);
    SetAlarm(true);
}

static void Finished_OnTick()
{
    g_Ticking_counter = (g_Ticking_counter + 1) % SECOND_DIVIDER;
    if (!g_Ticking_counter) {
        g_Seconds_counter = (g_Seconds_counter + 1) % FINISHED_STATE_TIMEOUT;
        if (!g_Seconds_counter) {
            ToState(STATE_IDLE);
        }
    }
}

static void Finished_OnButton(enum Button btn, enum ButtonAction action)
{
    ToState(STATE_IDLE);
    OnButton(btn, action);
}

static uint8_t g_UpdPause = 10;
static uint8_t g_UpdStep = 1;
static uint8_t g_ModCounter = 0;

static inline int EditStateModifier()
{
    return g_State == STATE_EDIT_PLUS ? 1 : -1;
}

static void Edit_OnEnter()
{
    g_UpdPause = 8;
    g_UpdStep = 1;
    g_Ticking_counter = 0;
    g_ModCounter = 0;
    DoEdit(g_UpdStep * EditStateModifier());
}

static void Edit_OnTick()
{
    ++g_Ticking_counter;

    if (g_Ticking_counter % g_UpdPause)
    {
        return;
    }
    ++ g_ModCounter;
    if (g_ModCounter % 10)
    {
        if (g_UpdPause > 2)
        {
            g_UpdPause /= 2;
        }
        else
        {
            if (g_UpdStep < 10)
            {
                g_UpdStep *= 2;
                if (g_UpdStep > 10)
                {
                    g_UpdStep = 10;
                }
            }
        }
    }
    DoEdit(g_UpdStep * EditStateModifier());
}

static void Edit_OnButton(enum Button btn, enum ButtonAction action)
{
    if (action != BTN_RELEASED)
    {
        return;
    }
    if ((g_State == STATE_EDIT_PLUS && btn == BTN_PLUS) ||
        (g_State == STATE_EDIT_MINUS && btn == BTN_MINUS))
    {
        SaveEeprom();
        ToState(STATE_IDLE);
    }
}

#ifdef MODE_SEGMENTS_PROBE
static uint8_t g_probedSegment = 0;
static void ProbeSegment_OnEnter()
{
    g_probedSegment = 0;
    PrintSegment(g_probedSegment);
}

static void ProbeSegment_OnTick()
{
}

static void ProbeSegment_OnButton(enum Button btn, enum ButtonAction action)
{
    g_probedSegment = (g_probedSegment + 1) % 8;
    PrintSegment(g_probedSegment);
}
#endif
