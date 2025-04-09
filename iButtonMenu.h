#ifndef __IBUTTON_MENU_H__
#define __IBUTTON_MENU_H__

#include "MenuItemInterface.h"

class IBUTTONMenu : public MenuItemInterface {
public:
    void optionsMenu(void);
    void draw(void);
    String getName(void);

private:
    String _name = "iButton";
};

#endif
