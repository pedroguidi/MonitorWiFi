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
  Serial.println("Configuração WiFi apagada. Reinicie o dispositivo.");
}


void setupHotspot() {
  WiFi.softAP("MonWiFi_Config", "12345678");
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", "<form action='/save' method='POST'>SSID: <input name='ssid'><br>Senha: <input name='password'><br><input type='submit'></form>");
  });
  server.on("/save", HTTP_POST, []() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
      saveWiFiConfig(server.arg("ssid").c_str(), server.arg("password").c_str());
      server.send(200, "text/html", "Configuração salva! Reiniciando o dispositivo.");
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
