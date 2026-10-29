#ifndef APPS_NOTES_APP_H
#define APPS_NOTES_APP_H

#include "kernel/app_base.h"
#include <vector>

struct NoteItem {
    uint32_t id;
    String title;
    String content;
    String date_str;
};

class NotesApp : public AppBase {
public:
    void on_enter() override;
    void on_exit() override;
    void on_update() override;
    void on_draw() override;
    bool on_touch(int16_t tx, int16_t ty, bool is_new_press) override;
    void on_nav_action() override;
    const char* get_title() const override { return "NOTAS"; }

    static void load_notes();
    static void save_notes();

private:
    int16_t scroll_offset;
    int16_t viewing_note_idx; // -1 si está en la lista principal

    void draw_list_view();
    void draw_detail_view();
};

#endif // APPS_NOTES_APP_H
