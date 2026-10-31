# broker-mqtt-iot

Control de variables críticas (temperatura y humedad) en las etapas de secado y almacenamiento de bioproductos de hoja de coca. Integra captura con ESP32, telemetría vía MQTT, procesamiento de series temporales mediante Telegraf e InfluxDB, y visualización con alertas instantáneas de baja latencia usando Grafana y Telegram

## Integrantes
* Valentina Arciniegas
* Davinson Tulande
* Carlos Daza
* Andres Coral

## Configuración de Mosquitto

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

```
