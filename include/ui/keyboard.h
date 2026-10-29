#ifndef UI_KEYBOARD_H
#define UI_KEYBOARD_H

#include <Arduino.h>

typedef void (*KeyboardDoneCallback)(bool saved);

void keyboard_init();
void keyboard_open(String* target, const char* title, KeyboardDoneCallback on_done = nullptr);
void keyboard_close();
bool keyboard_is_active();
void keyboard_draw();
bool keyboard_handle_touch(int16_t tx, int16_t ty);

#endif // UI_KEYBOARD_H
