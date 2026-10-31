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

### Archivo de Configuración 

se debe editar el archivo principal de configuración de Mosquitto  para habilitar la interfaz de red y la autenticación por contraseña:

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

Se aplica los cambios reiniciando el servicio de Mosquito `/etc/mosquitto/mosquitto.conf` y configurar para que inicie automáticamente junto con el servidor:

```bash
# Habilitar e iniciar el servicio
sudo systemctl enable mosquitto
sudo systemctl restart mosquitto

# Comprobar estado del servicio
sudo systemctl status mosquitto
```
### Verificación con Mqtt Explorer

Se debe descargar MQTT Explorer, al abrir la aplicación se debe configurar la conexión de la siguiente manera:

![MQTT_Explorer_Conexion](docs/MQTT_Explorer_Conexion.png)

una vez se realice la conexión se puede realizar un envio MQTT de la siguiente manera:

![MQTT_Explorer_EnviarDatos](docs/MQTT_Explorer_EnviarDatos.png)

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
### Lógica del Firmware y Publicación

* Se inicia el sensor DHT11
* Establece conexión a la red WiFi mediante las credenciales almacenadas.
* Para asegurar la validez de los datos de trazabilidad, el ESP32 obtiene la fecha y hora precisa desde la red antes de publicar cualquier métrica.
* Asigna los parámetros de conexión al cliente MQTT (PubSubClient)
* Evalúa constantemente la conexión al broker. Si la sesión cae, activa la rutina `reconnect()` que reintenta la autenticación cada 5 segundos.
* Utiliza `millis()` para muestrear y enviar datos cada 15 segundos `(interval = 15000)`, evitando el uso de llamadas `delay()` que detengan la ejecución.
* Lee la temperatura (°C) y la humedad (%). Si la lectura falla o arroja un valor no válido `(isnan)`, aborta el envío para evitar registrar datos corruptos.
* Envia los datos en Formato JSON de la siguiente manera:
  
```bash
{
  "id_sensor": "ESP32",
  "temperatura": 22.4,
  "humedad": 65.0,
  "lote": "LOTE-2026-01",
  "proceso": "SECADO_PASTOS",
  "fecha_hora": "2026-10-30 21:45:00"
}
```
publica el tópico `esp32/values`.

## 3. Prueba de conectividad MQTT -ESP32

Para verificar que el broker Mosquitto  en el servidor EC2 está recibiendo correctamente las métricas enviadas por el ESP32,se puede suscribir en tiempo real al tópico mediante la herramienta CLI de Mosquitto.

```bash
mosquitto_sub -h localhost -p 1883 -u "esp32_user" -P "1234" -t "esp32/values" -v
```

### Guardar CSV

se puede guardar el csv de la siguiente manera:

```bash
mosquitto_sub -h localhost -p 1883 -u "esp32_user" -P "1234" -t "esp32/values" >nombre_base_de_datos.csv
```

## 4.Configuración de InfluxDB

Para que la telemetría enviada por la ESP32 a través de MQTT pueda ser almacenada y consultada en tiempo real por Grafana, se utiliza InfluxDB como motor de base de datos especializado en series temporales. Se debe hacer la instalación en la red privada ya que se enfoca hacia la seguridad, la eficiencia y las buenas prácticas de arquitectura. 

### Requisitos Previos

Se debe realizar la conexión a la segunda instancia que esta en una red privada por medio de la primera instancia, esta permite recibir las conexiones SSH autorizadas desde el exterior (red pública) y reenviarlas de forma segura hacia la interfaz de red privada  de la segunda instancia, evitando exponer esta última directamente a Internet.

Asegurarse de tener habilitado el puerto correspondiente en las reglas del Security Group de AWS y en el firewall del servidor en red publica. Para este caso se habilita el puerto `8086` permitido únicamente desde la IP Privada de la Instancia 1.

```bash
# Actualizar repositorios e instalar dependencias básicas
apt-get update && apt-get install -y curl gnupg2

# Agregar la clave GPG oficial de InfluxData
wget -q https://repos.influxdata.com/influxdata-archive_compat.key
echo "39392622605745636582954638e33d4576d87a3e0c0073cd05943a7366367565 influxdata-archive_compat.key" | sha256sum -c && cat influxdata-archive_compat.key | gpg --dearmor | tee /etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg > /dev/null

#  Agregar el repositorio a las fuentes de APT
echo 'deb [signed-by=/etc/apt/trusted.gpg.d/influxdata-archive_compat.gpg] https://repos.influxdata.com/debian stable main' | tee /etc/apt/sources.list.d/influxdata.list

# Instalar únicamente el paquete InfluxDB (incluye motor y CLI 'influx')
apt-get update && apt-get install -y influxdb
```
### Control del Servicio

Habilitar InfluxDB para que se ejecute automáticamente cada vez que la instancia EC2 se reinicie:

```bash
# Iniciar el servicio
sudo systemctl start influxdb

# Habilitar el inicio automático
sudo systemctl enable influxdb

# Verificar que el servicio está activo (running)
sudo systemctl status influxdb
```
### Crear la Base de Datos

```bash
influx
-- Crear la base de datos para los registros del ESP32
CREATE DATABASE nombre_base_datos
-- Confirmar que la base de datos fue creada
SHOW DATABASES
```
## 5.Configuración de telegraf

Telegraf es un agente de software ligero y de código abierto (desarrollado por InfluxData) diseñado para la recolección, procesamiento y envío de datos y métricas,actúa como el "puente" o intermediario entre las fuentes que generan la información (sensores, brokers, servidores) y las bases de datos donde se almacena.

Su función dentro de la arquitectura distribuida es actuar como puente en tiempo real: se suscribe al broker local Mosquitto MQTT, procesa el payload JSON emitido por los dispositivos ESP32, y persiste la telemetría formateada en la base de datos InfluxDB alojada en la Instancia Privada (Instancia 2).

Se instala telegraf en la segunda instancia desde el repositorio, la información se encuentra e https://docs.influxdata.com/telegraf/v1/install/

```bash
curl --silent --location -O https://repos.influxdata.com/influxdata-archive.key
gpg --show-keys --with-fingerprint --with-colons ./influxdata-archive.key 2>&1 \
| grep -q '^fpr::*24C975CBA61A024EE1B631787C3D57159FC2F927:$' \
&& cat influxdata-archive.key \
| gpg --dearmor \
| sudo tee /etc/apt/keyrings/influxdata-archive.gpg > /dev/null \
&& echo 'deb [signed-by=/etc/apt/keyrings/influxdata-archive.gpg] https://repos.influxdata.com/debian stable main' \
| sudo tee /etc/apt/sources.list.d/influxdata.list
sudo apt-get update && sudo apt-get install telegraf
```
se instala el paquete de la siguiente manera:

```bash
apt-get update && apt-get install -y telegraf
```

### Archivo de Configuración 

se debe editar el archivo de configuración de telegraf `/etc/telegraf/telegraf.conf` para la entrada de datos por Mosquitto y  la salida hacia influxd

```bash
[[inputs.mqtt_consumer]]
         servers = ["Ip_privada_instancia_publica:1883"]
         topics =[ "esp32/values"]
         username = "esp32_user"
         password = "****"
         data_format ="json"
         json_string_fields = ["fecha_hora","id_sensor ", "temperatura","humedad","lote","proceso"]
         data_type ="float"
         json_timezone = "America/Bogota"
[[outputs.influxdb]]
        urls = ["Ip_privada_instancia_privada:8086"]
        database = "nombre_base_de_datos_creada_en_influxdb"
```
### Control del Servicio

Reiniciar y habilitar telegraf para que se ejecute automáticamente cada vez que la instancia EC2 se reinicie:

```bash
# Reiniciar el servicio
systemctl restart telegraf

# Iniciar el servicio
sudo systemctl start telegraf

# Habilitar el inicio automático
sudo systemctl enable telegraf

# Verificar que el servicio está activo (running)
sudo systemctl status telegraf
```
## 6. Verificación Base de datos Influxdb

Para verificar que si se guardo los datos enviados del ESP32 desde Mosquitto por medio de telegraf hacia Influxb se realiza el siguiente porceso

```bash
influx
-- ver dbs creados
show databases
-- usar db
use nombre_del_database
--realizar consulta
SELECT * FROM mqtt_consumer
```
Se debe observar algo como:

![Influx_Datos](docs/Influx_Datos.png)


