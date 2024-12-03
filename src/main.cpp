#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wi-Fi credentials
const char *ssid = "BTS_CIEL";
const char *password = "ERIR1234";

// LED GPIOs
const int ledH = 4;
const int ledB = 2;
const int ledG = 18;
const int ledD = 19;
String ledState;

// DS18B20 temperature sensor on GPIO 34
const int oneWireBus = 14;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Web server and WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variables for storing temperature data
String temperatureC;
String temperatureF;

// Function to notify all WebSocket clients with temperature data
void notifyClients(String data) {
    ws.textAll(data);
}

// Handle incoming WebSocket messages
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String message = (char *)data;
        if (message == "ON") {
            digitalWrite(ledB, HIGH);
        } else if (message == "OFF") {
            digitalWrite(ledB, LOW);
        }
    }
}

// WebSocket event handler
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("Client WebSocket #%u connected from: %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("Client WebSocket #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        default:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup() {
    Serial.begin(115200);

    // Initialize GPIOs for LEDs
    pinMode(ledH, OUTPUT);
    pinMode(ledB, OUTPUT);
    pinMode(ledG, OUTPUT);
    pinMode(ledD, OUTPUT);

    // Initialize temperature sensor
    sensors.begin();

    // Initialize SPIFFS
    if (!SPIFFS.begin()) {
        Serial.println("SPIFFS error...");
        return;
    }

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize WebSocket
    initWebSocket();

    // Serve index.html and style.css from SPIFFS
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/style.css", "text/css");
    });

    // LED control routes
    server.on("/haut", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(ledH, HIGH);
        digitalWrite(ledB, LOW);
        digitalWrite(ledG, LOW);
        digitalWrite(ledD, LOW);
        request->send(SPIFFS, "/index.html", String(), false);
    });
    server.on("/bas", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(ledB, HIGH);
        digitalWrite(ledH, LOW);
        digitalWrite(ledG, LOW);
        digitalWrite(ledD, LOW);
        request->send(SPIFFS, "/index.html", String(), false);
    });
    server.on("/droite", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(ledD, HIGH);
        digitalWrite(ledH, LOW);
        digitalWrite(ledB, LOW);
        digitalWrite(ledG, LOW);
        request->send(SPIFFS, "/index.html", String(), false);
    });
    server.on("/gauche", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(ledG, HIGH);
        digitalWrite(ledD, LOW);
        digitalWrite(ledH, LOW);
        digitalWrite(ledB, LOW);
        request->send(SPIFFS, "/index.html", String(), false);
    });

    // Start server
    server.begin();
}

void loop() {
    // Request temperature from DS18B20 sensor
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    float tempF = sensors.getTempFByIndex(0);
    temperatureC = String(tempC) + " ºC";
    temperatureF = String(tempF) + " ºF";

    // Log temperature readings to Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.print(" | ");
    Serial.println(temperatureF);

    // Notify WebSocket clients with temperature data
    notifyClients("Temperature: " + temperatureC + " | " + temperatureF);

    delay(5000);  // Delay to control update frequency
}
