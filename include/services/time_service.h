#ifndef SERVICES_TIME_SERVICE_H
#define SERVICES_TIME_SERVICE_H

#include <Arduino.h>
#include <time.h>

void time_service_init();
void time_service_update();
bool time_service_sync_ntp();
bool time_service_is_synced();

String time_service_get_time_str(bool with_seconds = false);
String time_service_get_date_str();
String time_service_get_full_date_str();

time_t time_service_get_epoch();
struct tm time_service_get_tm();
void time_service_set_time(int year, int month, int day, int hour, int minute, int second);

#endif // SERVICES_TIME_SERVICE_H
