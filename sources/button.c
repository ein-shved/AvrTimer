#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

enum ButtonState {
    BTN_INIT = 0,
    BTN_DEBONCE = _BV(0),
    BTN_OFF = _BV(1),
    BTN_ON = _BV(2),
    BTN_ON_DEBONCE = BTN_ON | BTN_DEBONCE,
    BTN_ON_HOLD = _BV(3),
    BTN_ON_MASK = BTN_ON | BTN_ON_HOLD,
    BTN_OFF_DEBONCE_ON = BTN_OFF | BTN_ON | BTN_DEBONCE,
    BTN_OFF_DEBONCE_HOLD = BTN_OFF | BTN_ON_HOLD | BTN_DEBONCE,
};

struct ButtonInfo {
    enum ButtonState state;
    uint8_t ticks_count;
} g_buttons [BTN_NUM] = { 0 };

static void CheckButton(bool gpioOn, struct ButtonInfo *btn);
static void TickButton(bool gpioOn, struct ButtonInfo *btn);
void OnGpio(void)
{
    CheckButton(MASK_BTN_RUN, &g_buttons[BTN_RUN]);
    CheckButton(MASK_BTN_PLUS, &g_buttons[BTN_PLUS]);
    CheckButton(MASK_BTN_MINUS, &g_buttons[BTN_MINUS]);
}

void OnGpioTick(void)
{
    TickButton(MASK_BTN_RUN, &g_buttons[BTN_RUN]);
    TickButton(MASK_BTN_PLUS, &g_buttons[BTN_PLUS]);
    TickButton(MASK_BTN_MINUS, &g_buttons[BTN_MINUS]);
}

static inline void ToBtnState(struct ButtonInfo *btn, enum ButtonState state)
{
    enum Button btnId = btn - g_buttons;
    if (state | BTN_DEBONCE)
    {
        btn->ticks_count = 0;
    }
    switch (state)
    {
    case BTN_OFF:
        OnButton(btnId, btn->state & BTN_ON ? BTN_CLICKED : BTN_RELEASED);
        break;
    case BTN_ON_HOLD:
        OnButton(btnId, BTN_HOLD);
        break;
    default:
        break;
    }
    btn->state = state;
}


static void CheckButton(bool gpioOn, struct ButtonInfo *btn)
{
    if (gpioOn) {
        switch (btn->state) {
        case BTN_INIT:
        case BTN_OFF:
            ToBtnState(btn, BTN_ON_DEBONCE);
            break;
        default:
            break;
        }
    } else {
        switch (btn->state) {
        case BTN_ON_HOLD:
            ToBtnState(btn, BTN_OFF_DEBONCE_HOLD);
            break;
        case BTN_ON:
            ToBtnState(btn, BTN_OFF_DEBONCE_ON);
            break;
        default:
            break;
        }
    }
}

static void TickButton(bool gpioOn, struct ButtonInfo *btn)
{
    ++btn->ticks_count;
    if (btn->state & BTN_DEBONCE)
    {
        if (btn->ticks_count < DEBONCE_TICS)
        {
            return;
        }
        enum ButtonState newState =
            btn->state & BTN_OFF
                ? (gpioOn
                    ? btn->state & BTN_ON_MASK
                    : BTN_OFF)
                : (gpioOn
                    ? BTN_ON
                    : BTN_OFF);
        ToBtnState(btn, newState);
    }
    else if (btn->state == BTN_ON)
    {
        if (btn->ticks_count < HOLD_TICS)
        {
            return;
        }
        ToBtnState(btn, BTN_ON_HOLD);
    };
}
