# ESP32-PDA (PocketOS) 📱✨

Sistema operativo ligero tipo **PDA (Personal Digital Assistant)** desarrollado para la placa **ESP32-2432S028 ("Cheap Yellow Display" / CYD)** con pantalla táctil TFT a color de 2.8 pulgadas.

Inspirado en la ergonomía y estética de los dispositivos clásicos tipo **PalmOS / PocketPC**, pero potenciado con conectividad WiFi moderna, sincronización horaria automática por internet (NTP) y persistencia de datos en memoria flash (LittleFS).

---

## 🌟 Características Principales

### 1. 📅 Calendario & Agenda
- Visualización de calendario mensual completo con navegación rápida entre meses y años.
- Marcadores visuales de días con eventos pendientes y resalte del día actual.
- Creación de eventos con título y hora usando el teclado virtual en pantalla.
- Almacenamiento persistente en formato JSON en la memoria flash interna.
- Borrado rápido de eventos pasados.

### 2. 📝 Bloc de Notas (Notes & Memos)
- Lista organizada de notas creadas con extracto y fecha de guardado.
- Visor a pantalla completa de notas largas.
- **Teclado virtual táctil QWERTY completo**:
  - Mayúsculas/minúsculas (Shift).
  - Selector de números y símbolos (`?123`).
  - Barra espaciadora y tecla de borrado.
  - Previsualización en vivo con cursor.

### 3. ⏰ Reloj Digital, NTP & Cronómetro
- Pantalla de reloj de mesa con hora digital en gran formato (`HH:MM:SS`) y fecha completa en español.
- Botón de sincronización con servidores horarios mundiales (**NTP**) vía WiFi.
- **Cronómetro integrado** con precisión de décimas de segundo (iniciar, pausar y reiniciar).

### 4. 🔢 Calculadora de Bolsillo
- Interfaz táctil de calculadora retro con botones de gran tamaño.
- Operaciones matemáticas fundamentales: suma, resta, multiplicación, división, porcentajes y cambio de signo (`+/-`).
- Visualización clara del operando y operador en curso.

### 5. ⛅ Pronóstico del Tiempo (Weather App)
- Consulta directa a la API REST de **Open-Meteo** (gratuita, sin necesidad de API keys).
- Iconografía gráfica dinámica (Sol, Nuboso, Lluvia, etc.).
- Temperatura actual (°C), velocidad del viento (km/h) y estado del cielo en tiempo real.

### 6. ⚙️ Panel de Control & Ajustes (Settings)
- **Gestor WiFi**: Escaneo de redes 2.4 GHz cercanas con indicador de intensidad de señal (RSSI), teclado para introducir contraseña y conexión en tiempo real.
- **Pantalla**: Regulador táctil de brillo de retroiluminación (PWM) y temporizador de reposo automático (1m, 3m, 5m, OFF).
- **Sonido**: Activación o silencio de los clics táctiles en el buzzer piezoeléctrico.
- **Información del Sistema**: Memoria RAM libre (Heap), uso del almacenamiento LittleFS, velocidad de CPU, dirección MAC y tiempo de funcionamiento.

---

## 🛠️ Hardware Compatible

Diseñado y calibrado específicamente para la placa **ESP32-2432S028 (CYD 2.8")**:

| Periférico | Pin ESP32 | Descripción |
| :--- | :---: | :--- |
| **TFT MOSI** | GPIO 13 | Bus SPI Pantalla (HSPI) |
| **TFT MISO** | GPIO 12 | Bus SPI Pantalla |
| **TFT SCLK** | GPIO 14 | Reloj SPI Pantalla |
| **TFT CS** | GPIO 15 | Chip Select Pantalla |
| **TFT DC** | GPIO 2 | Data / Command |
| **TFT Backlight** | GPIO 21 | Control de brillo PWM (Canal 7) |
| **Touch MOSI** | GPIO 32 | Panel táctil resistivo XPT2046 |
| **Touch MISO** | GPIO 39 | Panel táctil resistivo |
| **Touch CLK** | GPIO 25 | Reloj panel táctil |
| **Touch CS** | GPIO 33 | Chip Select panel táctil |
| **Buzzer** | GPIO 26 | Avisador acústico PWM (Canal 0) |
| **LED RGB** | GPIO 4, 16, 17 | LED indicador de estado (Active-LOW) |
| **LDR Sensor** | GPIO 34 | Sensor analógico de luz ambiental |

---

## 📂 Arquitectura de Software

El código está estructurado siguiendo principios modulares y desacoplados:

```
esp32pda/
├── include/
│   ├── config.h             # Pines, dimensiones, geometría y paleta RGB565
│   ├── hal/                 # Abstracción de hardware (Display, Buzzer, LED, Storage, Power)
│   ├── services/            # Servicios de fondo (WiFi, Time/NTP, Alarmas)
│   ├── ui/                  # Componentes visuales (Status Bar, Nav Bar, Keyboard, Widgets)
│   ├── kernel/              # Ciclo de vida y gestor de aplicaciones (AppBase, AppManager)
│   └── apps/                # Aplicaciones independientes (Launcher, Calendar, Notes, etc.)
├── src/                     # Implementación (.cpp) correspondiente
└── platformio.ini           # Configuración de compilación y dependencias
```

---

## 🚀 Compilación y Subida

### Requisitos:
- [PlatformIO Core](https://platformio.org/) o extensión de PlatformIO en VS Code.

### Comandos:
```bash
# Compilar el proyecto
pio run

# Flashear en la placa conectada por USB
pio run --target upload

# Monitor Serie a 115200 baudios
pio run --target monitor
```
