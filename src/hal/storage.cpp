#include "hal/storage.h"

bool storage_init() {
    // Intentar montar LittleFS; si falla (primera vez), formatear automáticamente
    if (!LittleFS.begin(true)) {
        log_e("Fallo critico al montar LittleFS");
        return false;
    }
    return true;
}

bool storage_write_file(const char* path, const String& content) {
    File f = LittleFS.open(path, "w");
    if (!f) return false;
    size_t written = f.print(content);
    f.close();
    return written == content.length();
}

String storage_read_file(const char* path) {
    if (!LittleFS.exists(path)) return String();
    File f = LittleFS.open(path, "r");
    if (!f) return String();
    String content = f.readString();
    f.close();
    return content;
}

bool storage_file_exists(const char* path) {
    return LittleFS.exists(path);
}

bool storage_delete_file(const char* path) {
    if (!LittleFS.exists(path)) return true;
    return LittleFS.remove(path);
}

size_t storage_total_bytes() {
    return LittleFS.totalBytes();
}

size_t storage_used_bytes() {
    return LittleFS.usedBytes();
}
