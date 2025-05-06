#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
SoftwareSerial g_serial(5, 6);  // RX, TX
em::EspAtManager g_esp_at_manager(g_serial);
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
  g_serial.begin(9600);
  auto ret = em::esp_at::ResultCode::kOK;

  Serial.println("module init");
  ret = g_esp_at_manager.Init();
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print("module init failed: ");
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  auto wifi = g_esp_at_manager.Wifi();
  Serial.println("wifi connecting...");
  ret = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print("wifi connect failed: ");
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  Serial.println("wifi connected");

  auto mqtt = g_esp_at_manager.Mqtt();
  mqtt.UserConfig(em::EspAtMqtt::ConnectionScheme::kMqttOverTcp, F("my_client_id"), F("my_user_name"), F("my_password"));
  Serial.println("mqtt connecting...");
  ret = mqtt.Connect(F("broker.emqx.io"), 1883);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print("mqtt connect failed: ");
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }
  Serial.println("mqtt connected");

  Serial.print("subscribe topic: ");
  Serial.println(kTestTopic);
  ret = mqtt.Subscribe(kTestTopic, 0);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print("mqtt subscribe failed: ");
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }
  Serial.println("mqtt subscribed");
}

void loop() {
  auto mqtt = g_esp_at_manager.Mqtt();
  const auto received_data = mqtt.Receive();
  if (received_data.content.length() > 0) {
    Serial.print("received topic: ");
    Serial.print(received_data.topic);
    Serial.print(", content length: ");
    Serial.print(received_data.content.length());
    Serial.print(", content: ");
    Serial.println(received_data.content);
  }

  static auto s_public_time = millis();
  if (millis() - s_public_time > 1000) {
    mqtt.Public(kTestTopic, String(F("timestamp:")) + s_public_time);
    s_public_time = millis();
  }
}