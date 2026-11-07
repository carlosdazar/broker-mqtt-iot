#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <pgmspace.h> // Memoria Flash

// --- Nombre de la Cosa en AWS IoT ---
const char THINGSNAME[] = "esp32-cocatrace";

// --- Configuración de Red WiFi ---
const char WIFI_SSID[]     = "WIFI_SSID";
const char WIFI_PASSWORD[] = "WIFI_PASSWORD";

// --- Identificadores ---
const char SENSOR_ID[] = "ESP32";
const char LOTE[]      = "LOTE-2026-01";
const char PROCESO[]   = "SECADO";

// Endpoint de AWS IoT
const char AWS_Iot_Endpoint[] = "AWS_Iot_Endpoint";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

// copiar certificado  AWS_CERT_CA

-----END CERTIFICATE-----
)EOF";

// Certificado del Dispositivo (Device Certificate)
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

// copiar certificado  AWS_CERT_CRT

-----END CERTIFICATE-----
)KEY";

// Clave Privada del Dispositivo (Device Private Key)
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

// copiar certificado  AWS_CERT_PRIVATE

-----END RSA PRIVATE KEY-----
)KEY";

#endif