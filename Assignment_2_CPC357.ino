#include <WiFi.h>              // ESP32 Wi-Fi library
#include <WiFiClientSecure.h>  // Secure connection library
#include <PubSubClient.h>      // MQTT library

// Wi-Fi credentials
const char* ssid = "OPPO A3x";
const char* password = "Nisa1234";

// Google Cloud Pub/Sub MQTT settings
const char* mqtt_server = "mqtt.googleapis.com";
const int mqtt_port = 8883;
const char* mqtt_topic = "projects/assignment-2-cpc357/topics/CPC357";

// Google Cloud Root CA certificate
const char* google_cloud_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDNTCCAh2gAwIBAgIUALN3Vc6nEZU/ZAzFkVkDyT/0ys0wDQYJKoZIhvcNAQEL
BQAwIjEMMAoGA1UEChMDVVNNMRIwEAYDVQQDEwlzbWFydHNhZmUwHhcNMjUwMTI1
MDUxNDQ3WhcNMzUwMTIzMDUxNDQ2WjAiMQwwCgYDVQQKEwNVU00xEjAQBgNVBAMT
CXNtYXJ0c2FmZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAJ+n5fxP
sBBOB2g1uIZwQv6ohzG9ROAji/P4yiug13McmQ5WiLo7m0XKFzyHzrK9Ej8pNIm/
6wNUN1xy0f7tkGhyTPl8bhhB4ocKM3VvM2Z7/ZjZJ+/YHuA0ErdewMC5nUACrKi0
+gAGSHaVKfnrWcJHj8aKnWkEHq7PB4Vt9I+YGvB7VowRKPNFY2Xx9jm05PPQ4bXj
RzCmW8aEUTga8ULkwc8v9Ym9Yg0UhePnhlhYNGhNoshObdYFKZfJynBJ060Vk295
z7y1pwBDDj2/wH6RSXpqWTOts20EQDVKpELE3t7ylz4xANTtqmWrFeiLF5lVxL7P
HQXGI9v0YOcSnl0CAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF
MAMBAf8wHQYDVR0OBBYEFCk27YkxkuqzPFUSY8pa4IW6zXyhMB8GA1UdIwQYMBaA
FCk27YkxkuqzPFUSY8pa4IW6zXyhMA0GCSqGSIb3DQEBCwUAA4IBAQAPoKFYVWBl
ZIVNQwWvRy1v1A0vQZy5X+/7lCekM/XEp0PMl0kKOuIF4oYpgJk3K9hK1PlQmLBE
R/cgI5So30fJclOg8WcKnx5k2pLhh9+fAiJhxIwzoIjrQLhwcG9yTSlfE/LZSOrG
X0LZwzfpMvhizK+rdUGdXrw6c4iAX2MFu8Vc2G4GIL2uSWlhYVG2RrV6gRdNn8VL
opINrMsauTRRhEK+HFAs/TgMCiU0F+fCwQXbT/He7OHcLa6PD869rzBZg1qdUn20
mFpYWV6VelxZtxKkeDCw+ODLQr5GRWwZTwOYM821tN8A0U0rC0Dzt60qB1VN0+ih
z2v53Ltt4VEn
-----END CERTIFICATE-----
)EOF";

// Global variables
WiFiClientSecure net;  // Secure client
PubSubClient client(net);

int pirPin = 22;  // GPIO pin for PIR sensor

// Function prototypes
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnect();
void publishToCloud(const char* message);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Configure MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  // Set Google Cloud root CA certificate
  net.setCACert(google_cloud_cert);

  // Initialize PIR sensor pin
  pinMode(pirPin, INPUT);
  Serial.println("Setup complete");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int pirState = digitalRead(pirPin);
  if (pirState == HIGH) {
    Serial.println("Motion detected!");
    publishToCloud("Motion detected");
  }
  delay(1000);
}

// MQTT callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "NodeMCU-32S-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT broker");
      client.subscribe(mqtt_topic);
    } else {
     Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// Publish message to Google Cloud
void publishToCloud(const char* message) {
  if (client.publish(mqtt_topic, message)) {
    Serial.println("Message published successfully");
  } else {
    Serial.println("Message publish failed");
  }
}
