#include "services/time_service.h"
#include "services/wifi_service.h"
#include <sys/time.h>
#include <esp_sntp.h>

static const char* NTP_SERVER1 = "pool.ntp.org";
static const char* NTP_SERVER2 = "time.google.com";
// Zona horaria de España/Europa Central con cambio de horario de verano automático (CET/CEST)
static const char* TIMEZONE_POSIX = "CET-1CEST,M3.5.0,M10.5.0/3";

static bool is_synced = false;
static unsigned long last_sync_check_ms = 0;
static const unsigned long SYNC_CHECK_INTERVAL_MS = 60000; // Cada minuto comprobar si podemos sincronizar

static const char* DAYS_SHORT[] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
static const char* MONTHS_SHORT[] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
static const char* MONTHS_FULL[] = { 
    "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", 
    "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre" 
};

void time_service_init() {
    configTzTime(TIMEZONE_POSIX, NTP_SERVER1, NTP_SERVER2);
}

void time_service_update() {
    if (millis() - last_sync_check_ms >= SYNC_CHECK_INTERVAL_MS) {
        last_sync_check_ms = millis();
        if (wifi_service_is_connected() && !is_synced) {
            time_service_sync_ntp();
        }
    }
}

bool time_service_sync_ntp() {
    if (!wifi_service_is_connected()) return false;

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 2000)) {
        if (timeinfo.tm_year > (2020 - 1900)) {
            is_synced = true;
            return true;
        }
    }
    return false;
}

bool time_service_is_synced() {
    return is_synced;
}

time_t time_service_get_epoch() {
    time_t now;
    time(&now);
    return now;
}

struct tm time_service_get_tm() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 50)) {
        time_t rawtime;
        time(&rawtime);
        localtime_r(&rawtime, &timeinfo);
    }
    return timeinfo;
}

String time_service_get_time_str(bool with_seconds) {
    struct tm t = time_service_get_tm();
    char buf[16];
    if (with_seconds) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
    } else {
        snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
    }
    return String(buf);
}

String time_service_get_date_str() {
    struct tm t = time_service_get_tm();
    char buf[24];
    int wday = t.tm_wday % 7;
    int mon = t.tm_mon % 12;
    snprintf(buf, sizeof(buf), "%s %02d %s", DAYS_SHORT[wday], t.tm_mday, MONTHS_SHORT[mon]);
    return String(buf);
}

String time_service_get_full_date_str() {
    struct tm t = time_service_get_tm();
    char buf[40];
    int mon = t.tm_mon % 12;
    snprintf(buf, sizeof(buf), "%d de %s de %04d", t.tm_mday, MONTHS_FULL[mon], t.tm_year + 1900);
    return String(buf);
}

void time_service_set_time(int year, int month, int day, int hour, int minute, int second) {
    struct tm tm_val;
    tm_val.tm_year = year - 1900;
    tm_val.tm_mon = month - 1;
    tm_val.tm_mday = day;
    tm_val.tm_hour = hour;
    tm_val.tm_min = minute;
    tm_val.tm_sec = second;
    tm_val.tm_isdst = -1;

    time_t t_of_day = mktime(&tm_val);
    struct timeval tv = { .tv_sec = t_of_day, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    is_synced = true;
}
