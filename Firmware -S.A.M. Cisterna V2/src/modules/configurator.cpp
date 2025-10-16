/*
 * S.A.M. Cisterna V2 - Configurator Module Implementation
 * FIXES: Removed Serial.begin(), proper server lifecycle management
 */

#include "configurator.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "../config/config.h"

// Static server instance (only created when needed)
static AsyncWebServer* configServer = nullptr;

void startConfigurationMode(WifiConfig& config) {
    Serial.println("[CONFIG] Starting configuration mode...");

    // Start Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASSWORD, CONFIG_AP_CHANNEL, false, CONFIG_AP_MAX_CONNECTIONS);

    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("[CONFIG] AP started: %s\n", CONFIG_AP_SSID);
    Serial.printf("[CONFIG] AP IP: %s\n", apIP.toString().c_str());
    Serial.printf("[CONFIG] Connect to WiFi '%s' with password '%s'\n", CONFIG_AP_SSID, CONFIG_AP_PASSWORD);
    Serial.println("[CONFIG] Then navigate to http://192.168.4.1");

    // Create server instance if it doesn't exist
    if (configServer == nullptr) {
        configServer = new AsyncWebServer(80);
    }

    // Root page - configuration form
    configServer->on("/", HTTP_GET, [&config](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<title>S.A.M. Cisterna V2 - Configurazione</title>";
        html += "<style>";
        html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
        html += ".container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }";
        html += "h1 { color: #333; text-align: center; }";
        html += "input[type='text'], input[type='password'] { width: 100%; padding: 10px; margin: 5px 0 15px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }";
        html += "input[type='submit'] { width: 100%; padding: 12px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }";
        html += "input[type='submit']:hover { background: #45a049; }";
        html += ".info { background: #e7f3fe; padding: 10px; border-left: 4px solid #2196F3; margin-bottom: 15px; }";
        html += "label { font-weight: bold; display: block; margin-top: 10px; }";
        html += ".restart-btn { background: #f44336; margin-top: 10px; }";
        html += ".restart-btn:hover { background: #da190b; }";
        html += "</style></head><body>";
        html += "<div class='container'>";
        html += "<h1>S.A.M. Cisterna V2</h1>";
        html += "<div class='info'>Configura le credenziali WiFi e l'indirizzo del server</div>";
        html += "<form method='POST' action='/submit'>";
        html += "<label>SSID Rete WiFi:</label>";
        html += "<input type='text' name='ssid' placeholder='Nome rete' required>";
        html += "<label>Password WiFi:</label>";
        html += "<input type='password' name='password' placeholder='Password rete' required>";
        html += "<label>Indirizzo IP Server:</label>";
        html += "<input type='text' name='ip' placeholder='192.168.1.100' required>";
        html += "<label>Porta Server:</label>";
        html += "<input type='text' name='port' placeholder='5000' required>";
        html += "<input type='submit' value='Salva Configurazione'>";
        html += "</form>";
        html += "<form method='POST' action='/restart'>";
        html += "<input type='submit' value='Riavvia Dispositivo' class='restart-btn'>";
        html += "</form>";
        html += "</div></body></html>";
        request->send(200, "text/html", html);
    });

    // Submit configuration
    configServer->on("/submit", HTTP_POST, [&config](AsyncWebServerRequest *request) {
        bool configChanged = false;

        if (request->hasParam("ssid", true)) {
            config.ssid = request->getParam("ssid", true)->value();
            configChanged = true;
        }
        if (request->hasParam("password", true)) {
            config.password = request->getParam("password", true)->value();
            configChanged = true;
        }
        if (request->hasParam("ip", true)) {
            config.ip = request->getParam("ip", true)->value();
            configChanged = true;
        }
        if (request->hasParam("port", true)) {
            config.port = request->getParam("port", true)->value();
            configChanged = true;
        }

        if (configChanged) {
            Serial.println("[CONFIG] Configuration received:");
            Serial.printf("  SSID: %s\n", config.ssid.c_str());
            Serial.printf("  IP: %s\n", config.ip.c_str());
            Serial.printf("  Port: %s\n", config.port.c_str());

            // Save to SPIFFS
            if (writeConfig(CONFIG_FILE_PATH, config)) {
                String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
                html += "<meta http-equiv='refresh' content='3;url=/'>";
                html += "<style>body{font-family:Arial;text-align:center;margin-top:50px;}</style></head><body>";
                html += "<h2>✓ Configurazione salvata!</h2>";
                html += "<p>Ritorno alla pagina principale...</p>";
                html += "</body></html>";
                request->send(200, "text/html", html);
            } else {
                request->send(500, "text/plain", "Errore nel salvataggio della configurazione");
            }
        } else {
            request->send(400, "text/plain", "Nessun parametro ricevuto");
        }
    });

    // Restart device
    configServer->on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<style>body{font-family:Arial;text-align:center;margin-top:50px;}</style></head><body>";
        html += "<h2>Riavvio in corso...</h2>";
        html += "<p>Attendere 10 secondi prima di riconnettersi</p>";
        html += "</body></html>";
        request->send(200, "text/html", html);

        // ✅ FIX: Proper server shutdown before restart
        delay(1000);
        stopConfigurationMode();
        delay(1000);
        ESP.restart();
    });

    // Start server
    configServer->begin();
    Serial.println("[CONFIG] Web server started");
}

void stopConfigurationMode() {
    // ✅ FIX: Proper server lifecycle management
    if (configServer != nullptr) {
        Serial.println("[CONFIG] Stopping configuration mode...");
        configServer->end();
        delete configServer;
        configServer = nullptr;
    }
    WiFi.softAPdisconnect(true);
}
