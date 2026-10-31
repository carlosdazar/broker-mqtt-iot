# broker-mqtt-iot

Control de variables críticas (temperatura y humedad) en las etapas de secado y almacenamiento de bioproductos de hoja de coca. Integra captura con ESP32, telemetría vía MQTT, procesamiento de series temporales mediante Telegraf e InfluxDB, y visualización con alertas instantáneas de baja latencia usando Grafana y Telegram

## Integrantes
* Valentina Arciniegas
* Davinson Tulande
* Carlos Daza
* Andres Coral

## 1. Configuración de Mosquitto 
Mosquitto es un intermediario de mensajes (message broker) de código abierto, ligero y de alto rendimiento que implementa el protocolo MQTT (Message Queuing Telemetry Transport).

### Requisitos Previos

Asegurarse de tener habilitado el puerto correspondiente en las reglas del Security Group de AWS y en el firewall del servidor en red publica. Para este caso se habilita el puerto `1883`.

### Instalación

En la instancia publica se debe instalar el servidor de Mosquitto junto con las utilidades de cliente para realizar pruebas directas:

```bash
sudo apt update
sudo apt install -y mosquitto mosquitto-clients
```

### Gestión de Usuarios y Autenticación

Para proteger el bróker y evitar accesos no autorizados se crean  credenciales específicas para la ESP32 

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd USUARIO
```
Se crea un usuario `esp32_user` con contraseña.

### Archivo de Configuración Principal 

se debe editar el archivo principal de configuración de Mosquitto para habilitar la interfaz de red y la autenticación por contraseña:

```bash
# Place your local configuration in /etc/mosquitto/conf.d/
# A full description of the configuration file is at
# /usr/share/doc/mosquitto/examples/mosquitto.conf

#pid_file /run/mosquitto/mosquitto.pid

persistence true
persistence_location /var/lib/mosquitto/

listener 1883 0.0.0.0
allow_anonymous false
password_file /etc/mosquitto/passwd

log_dest file /var/log/mosquitto/mosquitto.log

include_dir /etc/mosquitto/conf.d

```
### Control del Servicio

Se aplica los cambios reiniciando el servicio de Mosquito y configurar para que inicie automáticamente junto con el servidor:

```bash
# Habilitar e iniciar el servicio
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto

# Comprobar estado del servicio
sudo systemctl status mosquitto
```
### Verificación con Mqtt Explorer

Se debe descargar MQTT Explorer, al abrir la aplicación se debe configurar la conexión de la siguiente manera:


una vez se realice la conexión se puede realizar un envio MQTT de la siguiente manera:

## 2. Configuración de ESP32

El microcontrolador ESP32 está programado en C++ usando el framework de Arduino. Su función principal es leer los datos del sensor de temperatura y humedad (DHT11), sincronizar la hora local mediante un servidor NTP y enviar las mediciones periódicamente al servidor mediante el protocolo MQTT.

Sensor: DHT11 (Medición de temperatura y humedad ambiental).

Conexiones:

* DHTPIN: Data conectada al GPIO 19.
* VCC: 3.3V / 5V.
* GND: Tierra común.

### Archivo de Configuración (`config.h`)

El proyecto utiliza un archivo de cabecera centralizado (`config.h`) para gestionar las credenciales de red, dirección IP del servidor MQTT, parámetros de trazabilidad y configuración del hardware:

```bash
// --- Configuración de Red WiFi ---
#define WIFI_SSID     "NOMBRE RED WIFI 2.4"
#define WIFI_PASSWORD "CLAVE"

// --- Configuración del Broker MQTT ---
const char* mqtt_server = "18.209.235.164"; // Dirección IP de la instancia EC2
const int mqtt_port     = 1883;             // Puerto estándar de Mosquitto
const char* mqtt_user   = "esp32_user";     // Usuario autenticado
const char* mqtt_pass   = "****";           // Contraseña del usuario

// --- Metadatos de Trazabilidad y Proceso ---
#define SENSOR_ID     "ESP32"
#define LOTE          "LOTE-2026-01"
#define PROCESO       "SECADO_PASTOS"

// --- Configuración del Hardware ---
#define DHTPIN        19         // Pin GPIO19 conectado a la línea Data del sensor
#define DHTTYPE       DHT11      // Modelo del sensor de temperatura/humedad

#endif
```

