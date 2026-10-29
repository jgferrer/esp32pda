#ifndef SERVICES_WIFI_SERVICE_H
#define SERVICES_WIFI_SERVICE_H

#include <Arduino.h>
#include <vector>

struct ScannedNetwork {
    String ssid;
    int32_t rssi;
    bool is_open;
};

void wifi_service_init();
void wifi_service_update();
bool wifi_service_is_connected();
String wifi_service_get_ip();
int8_t wifi_service_get_rssi();
String wifi_service_get_current_ssid();

void wifi_service_connect(const String& ssid, const String& password);
void wifi_service_disconnect();
bool wifi_service_has_saved_credentials();

std::vector<ScannedNetwork> wifi_service_scan_networks();

#endif // SERVICES_WIFI_SERVICE_H
