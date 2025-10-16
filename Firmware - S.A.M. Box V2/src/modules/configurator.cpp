#include "../config/WifiConfig.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>

// Declare the global wifiConfig variable and AsyncWebServer here, only once
WifiConfig wifiConfig;
AsyncWebServer server(80);

// Write WiFi configuration to SPIFFS
void writeFile(const char *path, const WifiConfig &data) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }

    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["ssid"] = data.ssid;
    jsonDoc["password"] = data.password;
    jsonDoc["ip"] = data.ip;
    jsonDoc["port"] = data.port;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    if (file.print(jsonString)) {
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }

    file.close();
}

// Read WiFi configuration from SPIFFS
void readFile(const char *path, WifiConfig &data) {
    Serial.printf("Reading file: %s\r\n", path);

    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("- failed to open file for reading");
        return;
    }

    String jsonString = file.readString();
    DynamicJsonDocument jsonDoc(2048);
    DeserializationError error = deserializeJson(jsonDoc, jsonString);
    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        file.close();
        return;
    }

    // Apply the check to all relevant keys
    data.ssid = jsonDoc.containsKey("ssid") ? jsonDoc["ssid"].as<String>() : "default_ssid";
    data.password = jsonDoc.containsKey("password") ? jsonDoc["password"].as<String>() : "default_password";
    data.ip = jsonDoc.containsKey("ip") ? jsonDoc["ip"].as<String>() : "192.168.1.1";
    data.port = jsonDoc.containsKey("port") ? jsonDoc["port"].as<String>() : "5000";

    Serial.println("- read from file:");
    Serial.println("SSID: " + data.ssid);
    Serial.println("Password: " + data.password);
    Serial.println("IP: " + data.ip);
    Serial.println("Port: " + data.port);

    file.close();
}

// Start web configurator in AP mode
void configuratore(WifiConfig& config) {
    const char* ssid = "ESP32-Access-Point";
    const char* password = "password123";

    Serial.println("Starting configuration mode...");
    delay(500);

    // Start Access Point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Setup web server routes
    server.on("/", HTTP_GET, [&config](AsyncWebServerRequest *request) {
        String html = "<html><head><meta charset='UTF-8'><style>";
        html += "body { font-family: Arial; max-width: 500px; margin: 50px auto; padding: 20px; }";
        html += "input[type='text'], input[type='password'] { width: 100%; padding: 8px; margin: 8px 0; }";
        html += "input[type='submit'] { background: #4CAF50; color: white; padding: 10px 20px; border: none; cursor: pointer; }";
        html += "input[type='submit']:hover { background: #45a049; }";
        html += "h1 { color: #333; }";
        html += "</style></head><body>";
        html += "<h1>Configura S.A.M. Box V2</h1>";
        html += "<form method='post' action='/submit'>";
        html += "<p><label>SSID di rete:</label><br><input type='text' name='ssid' required /></p>";
        html += "<p><label>Password:</label><br><input type='password' name='password' required /></p>";
        html += "<p><label>IP Server:</label><br><input type='text' name='ip' required /></p>";
        html += "<p><label>Porta Server:</label><br><input type='text' name='port' required /></p>";
        html += "<br><input type='submit' value='Salva Configurazione' />";
        html += "</form>";
        html += "<br><hr><br>";
        html += "<form method='post' action='/restart'>";
        html += "<input type='submit' value='Riavvia Dispositivo' style='background: #f44336;' />";
        html += "</form></body></html>";
        request->send(200, "text/html", html);
    });

    server.on("/submit", HTTP_POST, [&config](AsyncWebServerRequest *request) {
        if (request->hasParam("ssid", true))
            config.ssid = request->getParam("ssid", true)->value();
        if (request->hasParam("password", true))
            config.password = request->getParam("password", true)->value();
        if (request->hasParam("ip", true))
            config.ip = request->getParam("ip", true)->value();
        if (request->hasParam("port", true))
            config.port = request->getParam("port", true)->value();

        Serial.println("Configuration received:");
        Serial.print("SSID: "); Serial.println(config.ssid);
        Serial.print("Password: "); Serial.println(config.password);
        Serial.print("IP: "); Serial.println(config.ip);
        Serial.print("Port: "); Serial.println(config.port);

        writeFile("/wifi_config.txt", config);

        String response = "<html><head><meta charset='UTF-8'></head><body>";
        response += "<h2>Configurazione salvata!</h2>";
        response += "<p>Il dispositivo si riavvierà tra 3 secondi...</p>";
        response += "<script>setTimeout(function(){ window.location.href='/restart'; }, 3000);</script>";
        response += "</body></html>";

        request->send(200, "text/html", response);
    });

    server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Riavvio in corso...");
        delay(1000);
        ESP.restart();
    });

    server.begin();
    Serial.println("Web server started. Connect to ESP32-Access-Point and open http://192.168.4.1");
}
