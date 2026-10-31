#ifndef CONFIG_H
#define CONFIG_H

// --- Configuración de Red WiFi (solo 2.4 GHz) ---
#define WIFI_SSID     "NOMBRE RED WIFI 2.4"
#define WIFI_PASSWORD "CLAVE"

// --- Configuración del Broker MQTT ---
const char* mqtt_server = "50.17.198.119";  // IP pública de la Instancia 1 (EC2)
const int mqtt_port     = 1883;             // Puerto estándar de Mosquitto
const char* mqtt_user   = "esp32_user";     // Usuario creado con mosquitto_passwd
const char* mqtt_pass   = "****";           // Contraseña del usuario
const char* mqtt_client_id = "ESP32_Client"; // Debe ser único por dispositivo

// --- Metadatos de Trazabilidad y Proceso ---
#define SENSOR_ID     "ESP32"
#define LOTE          "LOTE-2026-01"
#define PROCESO       "SECADO_PASTOS"

// --- Configuración del Hardware / Sensores ---
#define DHTPIN        19         // GPIO19 conectado a la línea Data del sensor
#define DHTTYPE       DHT11      // Modelo del sensor de temperatura/humedad

#endif
