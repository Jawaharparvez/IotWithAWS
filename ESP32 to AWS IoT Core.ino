#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi and AWS IoT Core credentials
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";
const char* mqtt_server = "your-iot-endpoint.iot.region.amazonaws.com";
const char* mqtt_topic = "room/temperature";

// DHT sensor setup
#define DHTPIN 4         // Pin connected to DHT sensor
#define DHTTYPE DHT11    // DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);

// Certificates
const char* ca_cert = "-----BEGIN CERTIFICATE-----\n...Your CA Cert...\n-----END CERTIFICATE-----";
const char* client_cert = "-----BEGIN CERTIFICATE-----\n...Your Client Cert...\n-----END CERTIFICATE-----";
const char* private_key = "-----BEGIN PRIVATE KEY-----\n...Your Private Key...\n-----END PRIVATE KEY-----";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Connect to Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

// Connect to MQTT broker (AWS IoT Core)
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client", "your-aws-iot-username", "your-aws-iot-password")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  dht.begin();
  
  espClient.setCACert(ca_cert);
  espClient.setCertificate(client_cert);
  espClient.setPrivateKey(private_key);
  
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Get temperature and humidity from DHT sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if the readings are valid
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish temperature and humidity data
  String payload = "{\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
  client.publish(mqtt_topic, payload.c_str());
  delay(2000);  // Delay for 2 seconds
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages if needed
}
