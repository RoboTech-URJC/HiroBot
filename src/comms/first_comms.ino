#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h" // File with SSID and passswd

const char* mqttServer = "broker.hivemq.com";
const int   mqttPort   = 1883;
const char* topic      = "/hirobot/pruebas";

WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
}

void connectMQTT() {
  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    String clientId = "ESP32-BOOL-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("Conectando a MQTT... ");

    if (client.connect(clientId.c_str())) {
      Serial.println("OK");
    } else {
      Serial.print("Fallo (rc=");
      Serial.print(client.state());
      Serial.println("), reintentando...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  connectWiFi();
  connectMQTT();

  Serial.println("  1  -> true");
  Serial.println("  0  -> false");
  Serial.println("  true / false");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Leer del puerto serie
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();   // quita espacios y \r\n

    String payload;
    bool valido = true;

    if (input == "1" || input.equalsIgnoreCase("true")) {
      payload = "true";
    } else if (input == "0" || input.equalsIgnoreCase("false")) {
      payload = "false";
    } else {
      valido = false;
      Serial.println("Valor no valido. Usa 1, 0, true o false.");
    }

    if (valido) {
      bool ok = client.publish(topic, payload.c_str());
      Serial.println("Publicado en " + String(topic) + " -> " + payload + (ok ? " [OK]" : " [ERROR]"));
    }
  }
}