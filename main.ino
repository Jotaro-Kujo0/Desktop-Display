#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#define TFT_CS 1
#define TFT_RST 2
#define TFT_DC 3
#define TFT_SCLK 4
#define TFT_MOSI 5
#define BUTTON_PIN 6
#define BATTERY_PIN A0

// Aggressive power saving settings
#define DEEP_SLEEP_TIMEOUT 60000    // 1 minute idle -> deep sleep
#define UPDATE_INTERVAL 2000        // Update every 2 seconds instead of 100ms
#define WIFI_TIMEOUT 10000          // WiFi connection timeout
#define MAX_RETRIES 3               // Max API call retries before sleep

char* SSID = "YOUR WIFI SSID";
const char* PASSWORD = "YOUR WIFI PASSWORD";
const char* CLIENT_ID = "YOUR CLIENT ID";
const char* CLIENT_SECRET = "YOUR CLIENT SECRET";

String lastArtist, lastTrackname;
unsigned long lastActivity = 0;
unsigned long lastUpdate = 0;
unsigned long scrollTimer = 0;
int scrollPos = 0;
int displayMode = 0;
int failedRequests = 0;
bool isPlaying = false;
bool wifiConnected = false;

Spotify sp(CLIENT_ID, CLIENT_SECRET);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void enterDeepSleep() {
    Serial.println("Entering deep sleep...");
    
    // Turn off display
    tft.fillScreen(ST77XX_BLACK);
    pinMode(TFT_CS, INPUT);
    pinMode(TFT_RST, INPUT);
    pinMode(TFT_DC, INPUT);
    
    // Disable WiFi and Bluetooth
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
    esp_bt_controller_disable();
    
    // Configure wake-up on button press
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_6, 0);
    
    // Optional: Wake up every 30 minutes to check for music
    esp_sleep_enable_timer_wakeup(30 * 60 * 1000000); // 30 min in microseconds
    
    esp_deep_sleep_start();
}

bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
        delay(250);
        Serial.print(".");
    }
    
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    
    if (wifiConnected) {
        // Set WiFi to low power mode
        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
        Serial.println("\nWiFi connected - Low power mode enabled");
    } else {
        Serial.println("\nWiFi connection failed");
        WiFi.mode(WIFI_OFF);
    }
    
    return wifiConnected;
}

void drawProgressBar(int progress) {
    int barWidth = 100;
    int barHeight = 6;
    int x = 30, y = 110;
    
    tft.drawRect(x, y, barWidth, barHeight, ST77XX_WHITE);
    tft.fillRect(x + 1, y + 1, (barWidth - 2) * progress / 100, barHeight - 2, ST77XX_GREEN);
}

void scrollText(String text, int x, int y, int maxWidth) {
    if (text.length() * 6 <= maxWidth) {
        tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        tft.setCursor(x, y);
        tft.print(text);
        return;
    }
    
    if (millis() - scrollTimer > 300) { // Slower scroll to save power
        String displayText = text.substring(scrollPos) + "   " + text.substring(0, scrollPos);
        tft.fillRect(x, y, maxWidth, 8, ST77XX_BLACK);
        tft.setCursor(x, y);
        tft.setTextColor(ST77XX_WHITE);
        tft.print(displayText.substring(0, maxWidth / 6));
        
        scrollPos = (scrollPos + 1) % text.length();
        scrollTimer = millis();
    }
}

void handleButton() {
    static unsigned long buttonPressStart = 0;
    static bool buttonPressed = false;
    
    bool currentState = !digitalRead(BUTTON_PIN);
    
    if (currentState && !buttonPressed) {
        buttonPressStart = millis();
        buttonPressed = true;
        lastActivity = millis();
    } else if (!currentState && buttonPressed) {
        if (millis() - buttonPressStart > 1000) { // Long press
            displayMode = (displayMode + 1) % 3;
            tft.fillScreen(ST77XX_BLACK);
        }
        buttonPressed = false;
    }
}

int getBatteryPercent() {
    int rawValue = analogRead(BATTERY_PIN);
    // Convert to percentage (adjust for your battery voltage)
    return map(rawValue, 2500, 4095, 0, 100);
}

void updateDisplay() {
    // Only update if enough time has passed
    if (millis() - lastUpdate < UPDATE_INTERVAL) {
        return;
    }
    
    String currentArtist = "";
    String currentTrackname = "";
    int progress = 0;
    
    // Only make API calls if WiFi is connected
    if (wifiConnected) {
        currentArtist = sp.current_artist_names();
        currentTrackname = sp.current_track_name();
        progress = sp.get_progress();
        
        // Check if music is playing
        isPlaying = !currentArtist.isEmpty() && !currentTrackname.isEmpty();
        
        if (isPlaying) {
            failedRequests = 0;
            lastActivity = millis(); // Reset sleep timer when music is active
        } else {
            failedRequests++;
        }
    }
    
    // Enter deep sleep if no music for too long
    if (failedRequests > MAX_RETRIES || !isPlaying) {
        if (millis() - lastActivity > DEEP_SLEEP_TIMEOUT) {
            enterDeepSleep();
        }
    }
    
    // Update display only if data changed
    if (lastArtist != currentArtist || lastTrackname != currentTrackname) {
        lastArtist = currentArtist;
        lastTrackname = currentTrackname;
        tft.fillScreen(ST77XX_BLACK);
    }
    
    switch(displayMode) {
        case 0