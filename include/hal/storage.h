#ifndef HAL_STORAGE_H
#define HAL_STORAGE_H

#include <Arduino.h>
#include <LittleFS.h>

bool storage_init();
bool storage_write_file(const char* path, const String& content);
String storage_read_file(const char* path);
bool storage_file_exists(const char* path);
bool storage_delete_file(const char* path);
size_t storage_total_bytes();
size_t storage_used_bytes();

#endif // HAL_STORAGE_H
