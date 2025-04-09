#include "ibutton.h"
#include "core/display.h"
#include "core/globals.h"
#include "core/mykeyboard.h"

void ibutton_read_run() {
    displayRedStripe("Reading...", TFT_WHITE, FGCOLOR);
    delay(2000);
    backToMenu();
}

void ibutton_write_run() {
    displayRedStripe("Writing...", TFT_WHITE, FGCOLOR);
    delay(2000);
    backToMenu();
}

void ibutton_clone_run() {
    displayRedStripe("Cloning...", TFT_WHITE, FGCOLOR);
    delay(2000);
    backToMenu();
}
