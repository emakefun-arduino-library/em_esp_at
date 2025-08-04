#include <Arduino.h>

#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
SoftwareSerial g_at_serial(5, 6);  // RX, TX
em::EspAtManager g_esp_at_manager(g_at_serial);
uint16_t ID() {
  const auto hour = atoi(__TIME__);
  const auto minute = atoi(__TIME__ + 3);
  const auto second = atoi(__TIME__ + 6);
  const auto seed = hour * 3600 + minute * 60 + second;
  randomSeed(seed);
  return random(0xFFFF);
}

const String kTestTopic = String("emakefun/sensor/") + ID() + "/timestamp";
}  // namespace

void setup() {
  Serial.begin(115200);
  g_at_serial.begin(9600);
  auto ret = em::esp_at::ResultCode::kOK;

  Serial.println(F("module init"));
  ret = g_esp_at_manager.Init();
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("module init failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  auto wifi = g_esp_at_manager.Wifi();
  Serial.println("wifi connecting...");
  ret = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("wifi connect failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  Serial.println("wifi connected");

  auto mqtt = g_esp_at_manager.Mqtt();
  mqtt.UserConfig(em::EspAtMqtt::ConnectionScheme::kMqttOverTcp, F("my_client_id"), F("my_user_name"), F("my_password"));
  Serial.println(F("mqtt connecting..."));
  ret = mqtt.Connect(F("broker.emqx.io"), 1883);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("mqtt connect failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }
  Serial.println(F("mqtt connected"));

  Serial.print(F("subscribe topic: "));
  Serial.println(kTestTopic);
  ret = mqtt.Subscribe(kTestTopic, 0);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("mqtt subscribe failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }
  Serial.println(F("mqtt subscribed"));
}

void loop() {
  auto& mqtt = g_esp_at_manager.Mqtt();
  mqtt.GetStream().setTimeout(100);
  auto received_data = mqtt.Receive();
  if (received_data.length > 0) {
    Serial.print(F("received topic: "));
    Serial.print(received_data.topic);
    Serial.print(F(", content length: "));
    Serial.print(received_data.length);
    Serial.print(F(", content: "));
    while (received_data.length > 0) {
      if (mqtt.GetStream().available() > 0) {
        Serial.print(char(mqtt.GetStream().read()));
        received_data.length--;
      }
    }
    Serial.println();
  }

  static auto s_public_time = millis();
  if (millis() - s_public_time > 500) {
    String content = String(F("test message with timestamp:")) + s_public_time;
    Serial.print(F("public content: "));
    Serial.println(content);
    mqtt.Public(kTestTopic, content);
    s_public_time = millis();
  }
}