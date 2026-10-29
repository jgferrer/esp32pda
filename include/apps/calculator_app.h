#ifndef APPS_CALCULATOR_APP_H
#define APPS_CALCULATOR_APP_H

#include "kernel/app_base.h"

class CalculatorApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    const char* get_title() const override { return "CALCULADORA"; }

private:
    double current_value;
    double operand_a;
    char pending_op;
    bool start_new_number;
    String display_buffer;

    void update_display();
    void calculate_result();
};

#endif // APPS_CALCULATOR_APP_H
