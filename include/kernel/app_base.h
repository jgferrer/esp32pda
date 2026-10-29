#ifndef KERNEL_APP_BASE_H
#define KERNEL_APP_BASE_H

#include <Arduino.h>
#include "config.h"

class AppBase {
public:
    virtual ~AppBase() {}
    virtual void on_enter() = 0;
    virtual void on_exit() {}
    virtual void on_update() = 0;
    virtual void on_draw() = 0;
    virtual bool on_touch(int16_t tx, int16_t ty, bool is_new_press) = 0;
    virtual void on_nav_action() {}
    virtual const char* get_title() const = 0;
};

#endif // KERNEL_APP_BASE_H
