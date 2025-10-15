#include <Arduino.h>

#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
constexpr uint8_t kLedPin = 13;

SoftwareSerial g_debug_serial(6, 5);  // rx,tx

em::EspAtManager g_esp_at_manager(Serial);

uint16_t ID() {
  const auto hour = atoi(__TIME__);
  const auto minute = atoi(__TIME__ + 3);
  const auto second = atoi(__TIME__ + 6);
  const auto seed = hour * 3600 + minute * 60 + second;
  randomSeed(seed);
  return random(0xFFFF);
}

constexpr String kMqttTopic = String("emakefun/sensor/") + ID() + "/timestamp";
}  // namespace

void setup() {
  pinMode(kLedPin, OUTPUT);

  g_debug_serial.begin(115200);
  Serial.begin(115200);

  auto result = em::esp_at::ResultCode::kOK;

  g_debug_serial.println(F("module init"));
  result = g_esp_at_manager.Init();
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("module init failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }

  auto wifi = g_esp_at_manager.Wifi();
  g_debug_serial.println("wifi connecting...");
  result = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("wifi connect failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }

  g_debug_serial.println("wifi connected");

  auto mqtt = g_esp_at_manager.Mqtt();
  mqtt.UserConfig(em::EspAtMqtt::ConnectionScheme::kMqttOverTcp, F("my_client_id"), F("my_user_name"), F("my_password"));
  g_debug_serial.println(F("mqtt connecting..."));
  result = mqtt.Connect(F("broker.emqx.io"), 1883);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("mqtt connect failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }
  g_debug_serial.println(F("mqtt connected"));

  g_debug_serial.print(F("subscribe topic: "));
  g_debug_serial.println(kMqttTopic);
  result = mqtt.Subscribe(kMqttTopic, 0);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("mqtt subscribe failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }
  g_debug_serial.println(F("mqtt subscribed"));
}

void loop() {
  auto& mqtt = g_esp_at_manager.Mqtt();
  mqtt.GetStream().setTimeout(100);
  auto received_data = mqtt.Receive();
  if (received_data.length > 0) {
    g_debug_serial.print(F("received topic: "));
    g_debug_serial.print(received_data.topic);
    g_debug_serial.print(F(", content length: "));
    g_debug_serial.print(received_data.length);
    g_debug_serial.print(F(", content: "));
    String content = "";
    uint16_t remaining_length = received_data.length;
    while (remaining_length > 0) {
      if (mqtt.GetStream().available() > 0) {
        content += char(mqtt.GetStream().read());
        remaining_length--;
      }
    }
    g_debug_serial.println(content);

    if (received_data.topic == kMqttTopic && content.length() == received_data.length) {
      if (content == "led on") {
        digitalWrite(kLedPin, HIGH);
        g_debug_serial.println(F("LED ON"));
      } else if (content == "led off") {
        digitalWrite(kLedPin, LOW);
        g_debug_serial.println(F("LED OFF"));
      }
    }
  }

  static auto s_public_time = millis();
  if (millis() - s_public_time > 500) {
    String content = (digitalRead(kLedPin) == HIGH) ? "led off" : "led on";
    g_debug_serial.print(F("public content: "));
    g_debug_serial.println(content);
    mqtt.Public(kMqttTopic, content);
    s_public_time = millis();
  }
}