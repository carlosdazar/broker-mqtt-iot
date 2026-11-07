#include <DHT.h>
#include <DHT_U.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "credentials.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "time.h"

#define DHTPIN 19
#define DHTTYPE DHT11  
#define LED_PIN 2

#define monitoreo_topic "esp32/values"     // Topic para enviar datos a AWS
#define suscribe_topic  "esp32/actuadores" // Topic para recibir ordenes

unsigned long lastMsg = 0;
const long interval = 5000;

// Inicializar conexión MQTT
WiFiClientSecure espClient;
PubSubClient esp32(espClient);

DHT dht(DHTPIN, DHTTYPE);

// Prototipos de funciones
void messageHandler(char* topic, byte* payload, unsigned int length);
void conncetAWS();
void publishMessage(float temperatura, float humedad);
void reconnect();

void conncetAWS(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configuración de certificados TLS
  espClient.setCACert(AWS_CERT_CA);
  espClient.setCertificate(AWS_CERT_CRT);
  espClient.setPrivateKey(AWS_CERT_PRIVATE);

  // Servidor broker, callback y buffer ampliado
  esp32.setServer(AWS_Iot_Endpoint, 8883);
  esp32.setCallback(messageHandler);
  esp32.setBufferSize(512); // Permite enviar JSONs grandes sin fallos

  Serial.println("Conectando a AWS IoT...");

  int retries = 0;
  while (!esp32.connect(THINGSNAME)) {
    Serial.print(".");
    delay(500);
    retries++;
    if (retries > 20) {
      Serial.println("\n[ERROR] Timeout al conectar con AWS IoT");
      return;
    }
  }

  // Suscribirse al topic del actuador
  esp32.subscribe(suscribe_topic);
  Serial.println("\nConectado a AWS IoT :D");
}

void publishMessage(float temperatura, float humedad){
  StaticJsonDocument<200> doc;

  time_t now = time(nullptr);

  doc["timestamp"] = now;
  doc["id_sensor"] = SENSOR_ID;
  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  doc["lote"] = LOTE;
  doc["proceso"] = PROCESO;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  // Publicar en monitoreo_topic ("esp32/values")
  if (esp32.publish(monitoreo_topic, jsonBuffer)) {
    Serial.println("Mensaje JSON publicado en MQTT exitosamente!");
    Serial.print("Payload: ");
    Serial.println(jsonBuffer);
  } else {
    Serial.println("[ERROR] Falló la publicación MQTT.");
  }
}

void messageHandler(char* topic, byte* payload, unsigned int length){
  Serial.print("Incoming topic: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extracción del campo "message"
  const char* message = doc["message"];

  if (message == NULL) {
    Serial.println("El JSON recibido no contiene el campo 'message'");
    return;
  }

  Serial.print("Comando recibido: ");
  Serial.println(message);

  if (strcmp(message, "ON") == 0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Se prendió el LED");
  }
  else if (strcmp(message, "OFF") == 0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Se apagó el LED");
  }
  else {
    Serial.println("Comando desconocido");
  }
}

void reconnect() {
  while (!esp32.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (esp32.connect(THINGSNAME)) {
      Serial.println("¡Conectado!");
      esp32.subscribe(suscribe_topic); // Volver a suscribirse al reconectar
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(esp32.state());
      Serial.println(" Reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(9600);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  conncetAWS();

  Serial.println("--- Estado Wi-Fi ---");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Conectado a: ");
  Serial.println(WIFI_SSID);

  // Configuración del servidor NTP para UTC-5
  configTime(-5 * 3600, 0, "pool.ntp.org");

  // Esperar a que la hora se sincronice con el servidor NTP
  struct tm timeinfo;
  Serial.print("Obteniendo hora NTP");
  while(!getLocalTime(&timeinfo)){
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada correctamente!");
}

void loop() {
  if (!esp32.connected()) {
    reconnect();
  }
  esp32.loop();

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