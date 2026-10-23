// Core-onafhankelijkheid: dezelfde code bouwt op Arduino-ESP32 1.0.6 en 2.0.x.
// Alleen wat tussen de twee cores verschilt staat hier, op één plek:
// de LittleFS-naam, de Wi-Fi-gebeurtenissen en het pad dat File::name() geeft.
#pragma once
#include <Arduino.h>
#if defined(__has_include)
#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#endif
#endif

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 2
#define CORE2 1
#include <LittleFS.h>
#define LITTLEFS LittleFS
#include <WiFi.h>
// Core 2 kent de oude SYSTEM_EVENT_*-namen nog, maar met andere nummers dan
// de gebeurtenissen die WiFi.onEvent levert: zet ze op de ARDUINO_EVENT_*.
#undef SYSTEM_EVENT_STA_CONNECTED
#undef SYSTEM_EVENT_STA_DISCONNECTED
#undef SYSTEM_EVENT_STA_GOT_IP
#undef SYSTEM_EVENT_AP_START
#undef SYSTEM_EVENT_AP_STACONNECTED
#undef SYSTEM_EVENT_AP_STADISCONNECTED
#define SYSTEM_EVENT_STA_CONNECTED      ARDUINO_EVENT_WIFI_STA_CONNECTED
#define SYSTEM_EVENT_STA_DISCONNECTED   ARDUINO_EVENT_WIFI_STA_DISCONNECTED
#define SYSTEM_EVENT_STA_GOT_IP         ARDUINO_EVENT_WIFI_STA_GOT_IP
#define SYSTEM_EVENT_AP_START           ARDUINO_EVENT_WIFI_AP_START
#define SYSTEM_EVENT_AP_STACONNECTED    ARDUINO_EVENT_WIFI_AP_STACONNECTED
#define SYSTEM_EVENT_AP_STADISCONNECTED ARDUINO_EVENT_WIFI_AP_STADISCONNECTED
#define WIFI_REDEN(info) ((info).wifi_sta_disconnected.reason)
#else
#include <LITTLEFS.h>
#define WIFI_REDEN(info) ((info).disconnected.reason)
#endif

// De watchdog: core 3 (IDF 5) neemt een config-struct in plaats van
// (seconden, paniek). Eén aanroep voor alle cores.
#include <esp_task_wdt.h>
inline void startWatchdog(uint32_t seconden) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_task_wdt_config_t cfg = { .timeout_ms = seconden * 1000, .idle_core_mask = 0, .trigger_panic = true };
  if (esp_task_wdt_reconfigure(&cfg) != ESP_OK) esp_task_wdt_init(&cfg);
#else
  esp_task_wdt_init(seconden, true);
#endif
}

// Watchdog voeden vanuit een taak die er (nog) niet aan hangt: core 1 negeert
// dat stil, core 3 logt bij elke aanroep een fout (tienduizenden regels per
// minuut). Alleen voeden als deze taak echt is aangemeld.
inline void voedWatchdog(void) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  if (esp_task_wdt_status(NULL) == ESP_OK) esp_task_wdt_reset();
#else
  esp_task_wdt_reset();
#endif
}

// File::name() geeft op core 1 "/naam", op core 2 "naam": altijd een pad.
inline String volledigPad(const char* naam) {
  return (naam && naam[0] == '/') ? String(naam) : String("/") + naam;
}
