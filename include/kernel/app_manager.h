#ifndef KERNEL_APP_MANAGER_H
#define KERNEL_APP_MANAGER_H

#include "kernel/app_base.h"

void app_manager_init();
void app_manager_register_app(AppID id, AppBase* app);
void app_manager_switch_to(AppID id);
AppID app_manager_get_current_id();
void app_manager_update();
void app_manager_handle_touch(int16_t tx, int16_t ty, bool is_new_press);
void app_manager_request_redraw();

#endif // KERNEL_APP_MANAGER_H
