#include <stdint.h>
#include <stdbool.h>

#define NOTIFY_THREASHOLD 5
#define DEFAULT_TIMEOUT 10
#define SECOND_DIVIDER 10
#define FINISHED_STATE_TIMEOUT 2
#define MSECS_PER_SEC 1000
#define DEBONCE_MSEC 100
#define HOLD_MSEC 200

#define MSEC_TO_TICS(MSECS) ((MSECS)/(MSECS_PER_SEC/SECOND_DIVIDER))

#define DEBONCE_TICS (MSEC_TO_TICS(DEBONCE_MSEC))
#if DEBONCE_TICS <= 0
#error Chosen debonce pause too little for chosen divider
#endif

#define HOLD_TICS (MSEC_TO_TICS(HOLD_MSEC))
#if HOLD_TICS <= 0
#error Chosen hold pause too little for chosen divider
#endif

#define MASK_BTN_RUN (PINC & _BV(5))
#define MASK_BTN_PLUS (PINC & _BV(4))
#define MASK_BTN_MINUS (PINC & _BV(3))
#define MASK_BTNS (PINC & (_BV(5) | _BV(4) | _BV(3)))

#define ALARM_PIN (_BV(4))

#define EEPROM_TIMER_POS ((uint16_t *)16)

enum State {
    STATE_IDLE = 0,
    STATE_TICKING,
    STATE_FINISHED,
    STATE_EDIT_PLUS,
    STATE_EDIT_MINUS,

    STATE_NUM,
};

enum Button {
    BTN_RUN = 0,
    BTN_PLUS,
    BTN_MINUS,

    BTN_NUM,
};

enum ButtonAction {
    BTN_CLICKED,
    BTN_HOLD,
    BTN_RELEASED,
};

void PrepareTimer();
void PreparePrinter();
void PrepareButtons();
void PrepareFsm();

void PutNumber(uint16_t num);
uint16_t GetNumber();
void SetNumberShow(bool show);

void ToState(enum State state);
void OnFsmTick(void);
void OnGpioTick(void);
void OnGpio(void);
void OnButton(enum Button btn, enum ButtonAction action);

void MainLoop();
void OnPrinterTick(uint8_t cnt);
