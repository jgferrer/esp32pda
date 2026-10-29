#include "services/wifi_service.h"
#include "hal/storage.h"
#include <WiFi.h>
#include <ArduinoJson.h>

static const char* WIFI_CONFIG_PATH = "/wifi.json";
static bool connecting = false;
static unsigned long connect_start_ms = 0;
static const unsigned long CONNECT_TIMEOUT_MS = 15000;
static String active_ssid = "";

void wifi_service_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Cargar credenciales guardadas si existen
    if (storage_file_exists(WIFI_CONFIG_PATH)) {
        String json = storage_read_file(WIFI_CONFIG_PATH);
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, json);
        if (!err) {
            const char* ssid = doc["ssid"];
            const char* pass = doc["password"];
            if (ssid && strlen(ssid) > 0) {
                wifi_service_connect(String(ssid), String(pass ? pass : ""));
            }
        }
    }
}

void wifi_service_update() {
    if (connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            connecting = false;
            active_ssid = WiFi.SSID();
        } else if (millis() - connect_start_ms > CONNECT_TIMEOUT_MS) {
            connecting = false;
            WiFi.disconnect();
        }
    }
}

bool wifi_service_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifi_service_get_ip() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return String("0.0.0.0");
}

int8_t wifi_service_get_rssi() {
    if (WiFi.status() == WL_CONNECTED) {
        return (int8_t)WiFi.RSSI();
    }
    return -100;
}

String wifi_service_get_current_ssid() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.SSID();
    }
    return active_ssid;
}

void wifi_service_connect(const String& ssid, const String& password) {
    if (ssid.length() == 0) return;
    active_ssid = ssid;
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
    connecting = true;
    connect_start_ms = millis();

    // Guardar credenciales en LittleFS
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["password"] = password;
    String out;
    serializeJson(doc, out);
    storage_write_file(WIFI_CONFIG_PATH, out);
}

void wifi_service_disconnect() {
    WiFi.disconnect();
    connecting = false;
    active_ssid = "";
    storage_delete_file(WIFI_CONFIG_PATH);
}

bool wifi_service_has_saved_credentials() {
    return storage_file_exists(WIFI_CONFIG_PATH);
}

std::vector<ScannedNetwork> wifi_service_scan_networks() {
    std::vector<ScannedNetwork> results;
    int n = WiFi.scanNetworks(false, true); // Síncrono rápido
    if (n > 0) {
        for (int i = 0; i < n && i < 15; ++i) {
            ScannedNetwork net;
            net.ssid = WiFi.SSID(i);
            net.rssi = WiFi.RSSI(i);
            net.is_open = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
            if (net.ssid.length() > 0) {
                results.push_back(net);
            }
        }
    }
    WiFi.scanDelete();
    return results;
}
