#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de Red WiFi ---
#define WIFI_SSID     "NOMBRE RED WIFI 2.4"
#define WIFI_PASSWORD "CLAVE"


const char* mqtt_server = "50.17.198.119"; //18.209.235.164
const int mqtt_port     = 1883;
const char* mqtt_user   = "esp32_user";
const char* mqtt_pass   = "****";

#define SENSOR_ID     "ESP32"
#define LOTE      "LOTE-2026-01"
#define PROCESO   "SECADO_PASTOS"

// --- Configuración del Hardware / Sensores ---
#define DHTPIN        19         // Pin D4
#define DHTTYPE       DHT11      // DHT11 

#endif