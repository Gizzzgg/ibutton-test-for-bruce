#include "iButtonMenu.h"
#include "core/display.h"
#include "modules/ibutton/ibutton.h"

void IBUTTONMenu::optionsMenu() {
    options = {
        {"Read",   [=]() { ibutton_read_run(); }},
        {"Write",  [=]() { ibutton_write_run(); }},
        {"Clone",  [=]() { ibutton_clone_run(); }},
        {"Main Menu", [=]() { backToMenu(); }}
    };

    delay(200);
    loopOptions(options, false, true, "iButton");
}

String IBUTTONMenu::getName() {
    return _name;
}

void IBUTTONMenu::draw() {
    tft.fillRect(iconX, iconY, 80, 80, BGCOLOR);
    tft.drawRoundRect(iconX + 10, iconY + 10, 60, 60, 10, FGCOLOR);
    tft.drawCentreString("iButton", iconX + 40, iconY + 40, SMOOTH_FONT);
}
