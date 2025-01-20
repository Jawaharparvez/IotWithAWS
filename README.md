# IotWithAWS
# ESP32 DHT Sensor with AWS IoT Core

This project demonstrates how to use an ESP32 microcontroller with DHT sensors to monitor temperature and humidity in a smart home environment, integrating the device with **AWS IoT Core** for cloud communication.

## Project Overview

The project reads temperature and humidity data from DHT sensors connected to the ESP32 and publishes the data to AWS IoT Core via MQTT. This enables remote monitoring of environmental conditions through AWS services.

## Prerequisites

- **AWS Account**: You’ll need an AWS account to set up AWS IoT Core.
- **ESP32 Board**: A compatible ESP32 development board.
- **DHT Sensor**: A DHT11 or DHT22 sensor to measure temperature and humidity.
- **Arduino IDE**: The code is written for the Arduino IDE, so you need to install it along with the necessary libraries for ESP32 and MQTT.

## Setting up AWS IoT Core

### 1. Set up AWS IoT Core

#### a. Create an AWS IoT Core account

If you don’t already have one, you’ll need to create an AWS account:
[Sign Up for AWS](https://aws.amazon.com/)

#### b. Navigate to AWS IoT Core Console

Once you’re logged in, search for **"IoT Core"** in the AWS Console, or you can directly go to the URL:
[AWS IoT Console](https://console.aws.amazon.com/iot/)

---

### 2. Create a "Thing" in AWS IoT Core

A "Thing" in AWS IoT Core represents a device like your ESP32 that will interact with AWS services.

#### a. Create the Thing

1. In the AWS IoT Console, go to **Manage** and click on **Things**.
2. Click on **Create a thing**.
3. You will be given an option to either create a single device or multiple devices at once. Choose **Create a single thing**.
4. Provide a name for your "Thing" (e.g., `RoomTemperatureSensorESP32`).
5. Optionally, you can assign a type and group, but for basic setups, this can be left empty.
6. Click **Next**.

#### b. Attach a Policy to the Thing (Optional)

You’ll need to create an IoT Policy for the thing that defines permissions. You can create this in the next step.

---

### 3. Create and Attach Policy

Policies in AWS IoT Core define what actions your device can perform (e.g., publishing to topics, subscribing to topics, etc.).

#### a. Create a Policy

1. Go to **Secure** in the IoT Core console and select **Policies**.
2. Click on **Create a policy**.
3. Define a policy with permissions like `iot:Connect`, `iot:Publish`, and `iot:Receive` as necessary.
4. Here’s an example of a basic policy JSON:

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "iot:Connect",
      "Resource": "*"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Publish",
      "Resource": "arn:aws:iot:region:account-id:topic/your-topic"
    },
    {
      "Effect": "Allow",
      "Action": "iot:Receive",
      "Resource": "arn:aws:iot:region:account-id:topic/your-topic"
    }
  ]
}
```
#### a. Required Downloads

- Device Certificate
- Public Key
- Private Key
- Amazon Root CA Certificate (if not downloaded, you can get it [here](https://www.amazontrust.com/repository/))

Ensure all these files are securely stored, as they will be used later for device authentication.

---

### 5. Set Up MQTT Topics

1. In the AWS IoT Core Console, navigate to **Test** > **MQTT test client**.
2. Define topics for the device to publish and subscribe to, such as:
   - Publish Topic: `esp32/dht/temperature`
   - Publish Topic: `esp32/dht/humidity`

---

## Setting Up ESP32 with Arduino IDE

### 1. Install Required Libraries

Open the Arduino IDE and install the following libraries:

- **ESP32 Board Library**:
  1. Go to **File** > **Preferences**.
  2. Add this URL in the **Additional Board Manager URLs**:  
     `https://dl.espressif.com/dl/package_esp32_index.json`
  3. Go to **Tools** > **Board** > **Boards Manager**, search for "ESP32" and install it.

- **DHT Sensor Library**:
  1. Go to **Sketch** > **Include Library** > **Manage Libraries**.
  2. Search for "DHT" and install the **DHT sensor library** by Adafruit.

- **PubSubClient Library**:
  1. Follow the same steps as above, but search for and install **PubSubClient**.

---

### 2. Code Implementation

#### a. Replace Placeholders

Replace the following placeholders in the code with your actual values:
- `WIFI_SSID`: Your Wi-Fi network name.
- `WIFI_PASSWORD`: Your Wi-Fi password.
- AWS IoT details:
  - Endpoint URL
  - Device certificate
  - Private key
  - Root CA certificate

#### b. Example Code

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Wi-Fi credentials
const char* WIFI_SSID = "Your_SSID";
const char* WIFI_PASSWORD = "Your_Password";

// AWS IoT Core details
const char* AWS_IOT_ENDPOINT = "your-endpoint.amazonaws.com";
const char* TOPIC_TEMPERATURE = "esp32/dht/temperature";
const char* TOPIC_HUMIDITY = "esp32/dht/humidity";

// Paths to certificates
const char* DEVICE_CERTIFICATE = "/path/to/device-cert.crt";
const char* DEVICE_PRIVATE_KEY = "/path/to/private-key.key";
const char* ROOT_CA_CERTIFICATE = "/path/to/root-ca.pem";

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Initialize Wi-Fi and MQTT client
WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Wi-Fi connected!");

  // Configure MQTT client
  espClient.setCACert(ROOT_CA_CERTIFICATE);
  espClient.setCertificate(DEVICE_CERTIFICATE);
  espClient.setPrivateKey(DEVICE_PRIVATE_KEY);
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Connect to AWS IoT Core
  while (!client.connected()) {
    Serial.println("Connecting to AWS IoT Core...");
    if (client.connect("ESP32_DHT")) {
      Serial.println("Connected to AWS IoT Core!");
    } else {
      Serial.print("Failed. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish temperature and humidity
  char tempStr[8];
  char humStr[8];
  dtostrf(temperature, 1, 2, tempStr);
  dtostrf(humidity, 1, 2, humStr);

  client.publish(TOPIC_TEMPERATURE, tempStr);
  client.publish(TOPIC_HUMIDITY, humStr);

  Serial.print("Temperature: ");
  Serial.println(tempStr);
  Serial.print("Humidity: ");
  Serial.println(humStr);

  delay(5000);  // Delay for 5 seconds
}
```


#### a. Required Downloads

- Device Certificate
- Public Key
- Private Key
- Amazon Root CA Certificate (if not downloaded, you can get it [here](https://www.amazontrust.com/repository/))

Ensure all these files are securely stored, as they will be used later for device authentication.

---

### 5. Set Up MQTT Topics

1. In the AWS IoT Core Console, navigate to **Test** > **MQTT test client**.
2. Define topics for the device to publish and subscribe to, such as:
   - Publish Topic: `esp32/dht/temperature`
   - Publish Topic: `esp32/dht/humidity`

---


####Testing the Setup
Upload the code to your ESP32 using the Arduino IDE.
Open the Serial Monitor to verify the connection to Wi-Fi and AWS IoT Core.
Use the MQTT test client in the AWS IoT Core Console to monitor the esp32/dht/temperature and esp32/dht/humidity topics for data updates.

####Conclusion
By integrating the ESP32 with AWS IoT Core, this project demonstrates a simple yet powerful way to monitor environmental conditions remotely. You can expand this setup further to include more sensors, create dashboards using AWS services like Amazon QuickSight, or even set up alerts using AWS Lambda.

