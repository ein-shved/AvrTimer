#include "timer.h"

#include <avr/io.h>


int main(void)
{
    PutNumber(DEFAULT_TIMEOUT);
    PrepareTimer();
    PrepareButtons();
    PreparePrinter();
    PrepareFsm();
    MainLoop();

    return 0;
}
