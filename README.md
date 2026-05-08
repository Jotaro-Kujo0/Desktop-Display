# Desktop Display — Spotify Display (Guıded Project)

A low-power physical Spotify controller and monitor built with an ESP32-C3. This version is optimized for battery life, featuring deep sleep capabilities, power-saving WiFi modes, and a customizable display interface.

---

## What it does

*   **Spotify Integration:** Shows current track, artist,album cover and live progress bar via the Spotify Web API.
*   **Power Optimized:** Uses Deep Sleep (~10μA), WiFi Modem Sleep, and disabled Bluetooth to maximize battery life.
*   **Smart Monitoring:** Built-in battery percentage monitoring and automatic sleep when idle.
*   **Physical Controls:** Multi-function button handling (long-press support) and dedicated playback buttons.
*   **Dynamic UI:** Supports multiple display modes and horizontal text scrolling for long titles.

---

## Hardware

### Bill of Materials

| Part | Description | Qty |
| :--- | :--- | :--- |
| **ESP32-C3 Pro Mini** | RISC-V, WiFi + BT5, USB-C, compact form factor | 1 |
| **ST7735 TFT LCD** | 1.8" SPI display, 128x160 resolution | 1 |
| **Tactile Buttons** | For playback control (Prev, Play/Pause, Skip) | 3 |
| **Function Button** | For mode switching and wake-up | 1 |
| **LiPo Battery** | 3.7V (1000mAh+ recommended) | 1 |
| **Resistors** | 10kΩ (for battery voltage divider) | 2 |
NOTE: The model has 6 holes for better customization. You can use 3 instead with a little change to code

### Wiring

#### Display & Inputs

| Component | ESP32-C3 Pin | Function |
| :--- | :--- | :--- |
| **TFT CS** | GPIO 1 | Chip Select |
| **TFT RST** | GPIO 2 | Reset |
| **TFT DC** | GPIO 3 | Data/Command |
| **TFT SCLK** | GPIO 4 | SPI Clock |
| **TFT MOSI** | GPIO 5 | SPI Data |
| **BUTTON** | GPIO 6 | Input (Internal Pullup) |
| **BATTERY** | A0 (GPIO 0) | Voltage Monitor (via divider) |
| **VCC/GND** | 3.3V / GND | Power Rail |

---

## Software Setup

### 1. Requirements
*   **Arduino IDE**: [Download here](https://www.arduino.cc/en/software)
*   **Board Support**: Install ESP32 by Espressif Systems (Select **ESP32C3 Dev Module**)

### 2. Libraries
Install the following via the Library Manager:
*   `Adafruit_GFX`
*   `Adafruit_ST7735`
*   `ArduinoJson`
*   `SpotifyEsp32` (Install manually as .zip from [GitHub](https://github.com/FinianLandes/SpotifyEsp32))

### 3. Configuration
Update your credentials in the firmware:

```cpp
char* SSID           = "YOUR_WIFI_NAME";
const char* PASSWORD       = "YOUR_WIFI_PASSWORD";
const char* CLIENT_ID     = "YOUR_SPOTIFY_CLIENT_ID";
const char* CLIENT_SECRET = "YOUR_SPOTIFY_CLIENT_SECRET";

// Power Settings
#define DEEP_SLEEP_TIMEOUT 60000   // 1 min idle -> sleep
#define UPDATE_INTERVAL    2000    // 2 sec refresh rate
