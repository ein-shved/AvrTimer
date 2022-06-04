#include <stdint.h>
#include <stdbool.h>

#define NOTIFY_THREASHOLD 5
#define DEFAULT_TIMEOUT 10
#define SECOND_DIVIDER 10
#define FINISHED_STATE_TIMEOUT 2
#define MSECS_PER_SEC 1000
#define DEBONCE_MSEC 100
#define HOLD_MSEC 200
#define SLEEP_TIMEOUT_SEC 10

#define MSEC_TO_TICS(MSECS) ((MSECS)/(MSECS_PER_SEC/SECOND_DIVIDER))
#define SEC_TO_TICS(SECS) ((SECS)*SECOND_DIVIDER)

#define DEBONCE_TICS (MSEC_TO_TICS(DEBONCE_MSEC))
#if DEBONCE_TICS <= 0
#error Chosen debonce pause too little for chosen divider
#endif

#define HOLD_TICS (MSEC_TO_TICS(HOLD_MSEC))
#if HOLD_TICS <= 0
#error Chosen hold pause too little for chosen divider
#endif

#define SLEEP_TIMEOUT_TICKS (SEC_TO_TICS(SLEEP_TIMEOUT_SEC))

#define MASK_BTN_RUN (!(PINC & _BV(4)))
#define MASK_BTN_PLUS (!(PINC & _BV(5)))
#define MASK_BTN_MINUS (!(PINC & _BV(3)))


#define SEGMENT_A   _BV(6)
#define SEGMENT_B   _BV(0)
#define SEGMENT_C   _BV(1)
#define SEGMENT_D   _BV(4)
#define SEGMENT_E   _BV(5)
#define SEGMENT_F   _BV(7)
#define SEGMENT_G   _BV(2)
#define SEGMENT_DP  _BV(3)

/*
 *   /===a===\
 *  ||       ||
 *  f        b
 *  ||       ||
 *   |===g===|
 *  ||       ||
 *  e        c
 *  ||       ||  dp
 *   \===d===/   *
 */

//#define MODE_SEGMENTS_PROBE

#define ALARM_PIN (_BV(4))

#define EEPROM_TIMER_POS ((uint16_t *)16)

enum State {
    STATE_IDLE = 0,
    STATE_TICKING,
    STATE_FINISHED,
    STATE_EDIT_PLUS,
    STATE_EDIT_MINUS,
#ifdef MODE_SEGMENTS_PROBE
    STATE_SEGMENTS_PROBE,
#endif

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
void GoSleep();
void SetAlarm(bool on);

#ifdef MODE_SEGMENTS_PROBE
void PrintSegment(uint8_t g_probedSegment);
#endif
