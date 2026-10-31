#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "config.h"
#include <ArduinoJson.h>
#include "time.h" // Librería nativa para NTP

#define topic "esp32/values"

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 15000; // Enviar datos cada 15 segundos

// Configuración de Servidor NTP y Zona Horaria
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000; // Ajuste GMT para tu zona (Ejemplo: -5 horas = -5 * 3600 = -18000)
const int daylightOffset_sec = 0;   // Ajuste horario de verano (si aplica)

// Función para obtener la hora actual en formato String
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error al obtener la hora NTP");
    return "N/A";
  }
  char timeStringBuff[30];
  // Formato: YYYY-MM-DD HH:MM:SS (Ejemplo: 2026-03-30 14:25:00)
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

void publishMessage(float temperatura, float humedad) {
  StaticJsonDocument<200> doc;
  
  doc["id_sensor"] = SENSOR_ID;
  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  doc["lote"] = LOTE;
  doc["proceso"] = PROCESO;
  doc["fecha_hora"] = getFormattedTime(); // <--- Campo con la fecha y hora agregada

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  client.publish(topic, jsonBuffer);
  Serial.println("Mensaje JSON publicado en MQTT exitosamente!");
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configurar la sincronización de la hora vía NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sincronizando hora NTP...");

  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32_Client", mqtt_user, mqtt_pass)) {
      Serial.println("¡Conectado!");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();

    if (isnan(temperatura) || isnan(humedad)) {
      Serial.println("Error al leer el sensor DHT!");
      return;
    }

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" °C | Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    publishMessage(temperatura, humedad);
  }
}