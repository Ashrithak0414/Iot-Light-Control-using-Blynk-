#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ThingSpeak.h>

/* --- NETWORK CONFIGURATION --- */
#define MY_SSID "YOUR_WIFI_SSID"
#define MY_PASS "YOUR_WIFI_PASSWORD"

/* --- THINGSPEAK CONFIGURATION --- */
unsigned long tsChannelNum = 0;
const char* tsWriteKey = "YOUR_WRITE_API_KEY";

/* --- HARDWARE SETTINGS --- */
#define DHT_DATA_PIN 13 // GPIO13 (D7)
#define SENSOR_MODEL DHT11

DHT dhtSensor(DHT_DATA_PIN, SENSOR_MODEL);
WiFiClient tsClient;

/* --- GLOBAL VARIABLES --- */
const long REFRESH_RATE = 20000; // Updated to 20s for better stability
unsigned long prevMillis = 0;

/**
* Validates and maintains WiFi connection
*/
void checkNetwork() {
if (WiFi.status() == WL_CONNECTED) return;

Serial.printf("\nAttempting connection to: %s\n", MY_SSID);
WiFi.begin(MY_SSID, MY_PASS);

int timeout = 0;
while (WiFi.status() != WL_CONNECTED && timeout < 20) {
delay(500);
Serial.print("#");
timeout++;
}

if (WiFi.status() == WL_CONNECTED) {
Serial.println("\n[SUCCESS] Network Active");
Serial.print("IP: ");
Serial.println(WiFi.localIP());
}
}

void setup() {
Serial.begin(115200);

// Initialize Peripherals
dhtSensor.begin();
ThingSpeak.begin(tsClient);

WiFi.mode(WIFI_STA);
checkNetwork();

Serial.println("--- System Telemetry Initialized ---");
}

void loop() {
unsigned long now = millis();

// Non-blocking timer for data transmission
if (now - prevMillis >= REFRESH_RATE) {
prevMillis = now;

// Data Acquisition
float h = dhtSensor.readHumidity();
float t = dhtSensor.readTemperature();

// Verification Logic
if (isnan(h) || isnan(t)) {
Serial.println("!! Sensor Read Error !!");
return;
}

Serial.printf("Temp: %.1f°C | Hum: %.1f%%\n", t, h);

// Verify connection before push
checkNetwork();

if (WiFi.status() == WL_CONNECTED) {
// Map data to ThingSpeak Fields
ThingSpeak.setField(1, t);
ThingSpeak.setField(2, h);
ThingSpeak.setStatus("Node Active - Telemetry Sent");

// Execute Transmission
int response = ThingSpeak.writeFields(tsChannelNum, tsWriteKey);

if (response == 200) {
Serial.println(">> Cloud Sync: OK");
} else {
Serial.printf(">> Sync Error: %d\n", response);
}
}
}
}