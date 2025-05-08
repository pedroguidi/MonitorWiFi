#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESP8266Ping.h>
#include <ESP8266WebServer.h>

#define LED_SUCCESS 5  // LED 1 - Sucesso (D1)
#define LED_FAIL 4     // LED 2 - Falha (D2)
#define LED_SOFTAP 2   // LED 3 - Modo SoftAP (D4)
#define RESET_BUTTON 0 // Pino do botão de reset (D3), configurado como PULL-UP

const char* host = "www.uol.com.br";
ESP8266WebServer server(80);
unsigned long previousMillisSuccess = 0;
unsigned long previousMillisFail = 0;
unsigned long previousMillisSoftAP = 0;
unsigned long previousMillisPing = 0;
unsigned long previousMillisReboot = 0;
const long intervalSuccess = 1700;
const long intervalFail = 1700;
const long intervalSoftAP = 1700;
const long intervalPing = 5000;
const long intervalReboot = 60000;
bool successState = LOW;
bool failState = LOW;
bool softAPState = LOW;
bool pingSuccess = false;
bool pingInProgress = false;

struct WiFiConfig {
  char ssid[32];
  char password[32];
};
WiFiConfig wifiConfig;

void saveWiFiConfig(const char* ssid, const char* password) {
  strncpy(wifiConfig.ssid, ssid, sizeof(wifiConfig.ssid));
  strncpy(wifiConfig.password, password, sizeof(wifiConfig.password));
  EEPROM.put(0, wifiConfig);
  EEPROM.commit();
}

void loadWiFiConfig() {
  EEPROM.get(0, wifiConfig);
  if (strlen(wifiConfig.ssid) == 0 || strlen(wifiConfig.password) == 0) {
    strcpy(wifiConfig.ssid, "");
    strcpy(wifiConfig.password, "");
  }
}

void clearWiFiConfig() {
  strcpy(wifiConfig.ssid, "");
  strcpy(wifiConfig.password, "");
  EEPROM.put(0, wifiConfig);
  EEPROM.commit();
}


void setupHotspot() {
  WiFi.softAP("MonWiFi_Config", "12345678");
  server.on("/", HTTP_GET, []() {
   server.send(200, "text/html", R"rawliteral(
                    <html>
                    <head>
                      <meta name='viewport' content='width=device-width, initial-scale=1'>
                      <style>
                        body {
                          font-family: Arial, sans-serif;
                          padding: 20px;
                          background-color: #f2f2f2;
                        }
                        form {
                          max-width: 300px;
                          margin: auto;
                          background: white;
                          padding: 15px;
                          border-radius: 10px;
                          box-shadow: 0 0 10px rgba(0,0,0,0.1);
                        }
                        h2 {
                          text-align: center;
                          color: #333;
                        }
                        input[type='text'], input[type='password'] {
                          width: 100%;
                          padding: 10px;
                          margin: 6px 0 12px;
                          border: 1px solid #ccc;
                          border-radius: 5px;
                        }
                        input[type='submit'] {
                          width: 100%;
                          padding: 10px;
                          background-color: #4CAF50;
                          color: white;
                          border: none;
                          border-radius: 5px;
                          cursor: pointer;
                        }
                        input[type='submit']:hover {
                          background-color: #45a049;
                        }
                      </style>
                    </head>
                    <body>
                      <h2>Monitor de WiFi</h2>
                      <h2>Registre o nome do Wi-Fi</h2>
                      <form action='/save' method='POST'>
                        <label>SSID:</label>
                        <input type='text' name='ssid'><br>
                        <label>Senha:</label>
                        <input type='password' name='password'><br>
                        <input type='submit' value='Conectar'>
                      </form>
                    </body>
                    </html>
                  )rawliteral");

  });
  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
      saveWiFiConfig(server.arg("ssid").c_str(), server.arg("password").c_str());
      server.send(200, "text/html", R"rawliteral(
                      <html>
                      <head>
                        <meta name='viewport' content='width=device-width, initial-scale=1'>
                        <style>
                          body {
                            font-family: Arial, sans-serif;
                            background-color: #f2f2f2;
                            display: flex;
                            justify-content: center;
                            align-items: center;
                            height: 100vh;
                            margin: 0;
                          }
                          .message-box {
                            background-color: white;
                            padding: 20px 30px;
                            border-radius: 10px;
                            box-shadow: 0 0 10px rgba(0,0,0,0.1);
                            text-align: center;
                          }
                        </style>
                      </head>
                      <body>
                        <div class="message-box">
                          <h2>Registrado!</h2>
                          <p>Reiniciando o dispositivo...</p>
                        </div>
                      </body>
                      </html>
                    )rawliteral");
      delay(3000);
      ESP.restart();
    }
  });
  server.begin();
}

void connectWiFi() {
  WiFi.begin(wifiConfig.ssid, wifiConfig.password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
}

void asyncPingCheck() {
  if (!pingInProgress) {
    pingInProgress = true;
    WiFiClient client;
    pingSuccess = client.connect(host, 80);
    Serial.println(pingSuccess ? "Conexão com a internet OK" : "Sem conexão com a internet");
    pingInProgress = false;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_SUCCESS, OUTPUT);
  pinMode(LED_FAIL, OUTPUT);
  pinMode(LED_SOFTAP, OUTPUT);
  pinMode(RESET_BUTTON, INPUT_PULLUP);
  EEPROM.begin(sizeof(WiFiConfig));
  loadWiFiConfig();

  if (digitalRead(RESET_BUTTON) == LOW) {
    clearWiFiConfig();
  }

  if (strlen(wifiConfig.ssid) > 0 && strlen(wifiConfig.password) > 0) {
    connectWiFi();
  }

  if (WiFi.status() != WL_CONNECTED) {
    setupHotspot();
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.getMode() != WIFI_STA) {
    if (currentMillis - previousMillisSoftAP >= intervalSoftAP) {
      previousMillisSoftAP = currentMillis;
      softAPState = !softAPState;
      digitalWrite(LED_SOFTAP, softAPState);
      digitalWrite(LED_SUCCESS, LOW);
      digitalWrite(LED_FAIL, LOW);
      Serial.println(softAPState ? "LED SOFTAP ligado" : "LED SOFTAP desligado");
    }
    if (currentMillis - previousMillisReboot >= intervalReboot) {
      previousMillisReboot = currentMillis;
      ESP.restart();
    }
  }

  if (currentMillis - previousMillisPing >= intervalPing) {
    previousMillisPing = currentMillis;
    asyncPingCheck();
  }


  if (WiFi.getMode() == WIFI_STA ) {
    digitalWrite(LED_SOFTAP, LOW);
    if (currentMillis - previousMillisSuccess >= intervalSuccess and pingSuccess) {
      previousMillisSuccess = currentMillis;
      successState = !successState;
      digitalWrite(LED_SUCCESS, successState);
      digitalWrite(LED_FAIL, LOW);
      Serial.println(successState ? "LED SUCCESS ligado" : "LED SUCCESS desligado");
    }

    if (currentMillis - previousMillisFail >= intervalFail and !pingSuccess) {
      previousMillisFail = currentMillis;
      failState = !failState;
      digitalWrite(LED_FAIL, failState);
      digitalWrite(LED_SUCCESS, LOW);
      Serial.println(failState ? "LED FAIL ligado" : "LED FAIL desligado");
    }
  }


  server.handleClient();
  
  if (digitalRead(RESET_BUTTON) == LOW) {
    clearWiFiConfig();
    delay(3000);
    ESP.restart();
  }
}
