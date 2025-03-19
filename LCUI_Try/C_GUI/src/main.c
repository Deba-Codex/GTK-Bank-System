// #include <stdio.h>
#include "LCUI.h"
int main(int argc, char *argv[]) {
    LCUI_Init();
    LCUI_Widget root = LCUIWidget_New("root");
    LCUI_Widget label = LCUIWidget_New("label");
    LCUIWidget_SetText(label, "Hello, LCUI!");
    LCUIWidget_Append(root, label);
    LCUIWidget_Show(root, true);
    LCUI_MainLoop();
    LCUIWidget_Destroy(root);
    LCUI_Destroy();
    return 0;
}