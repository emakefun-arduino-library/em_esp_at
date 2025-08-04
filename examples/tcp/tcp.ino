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
  Serial.println(F("wifi connecting..."));
  ret = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("wifi connect failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  Serial.println(F("wifi connected"));
  String ip;
  String gateway;
  String netmask;
  em::esp_at::ResultCode err = wifi.Ip(&ip, &gateway, &netmask);
  if (err == em::esp_at::ResultCode::kOK) {
    Serial.print("ip: ");
    Serial.println(ip);
    Serial.print("gateway: ");
    Serial.println(gateway);
    Serial.print("netmask: ");
    Serial.println(netmask);
  } else {
    Serial.print("wifi ip failed: ");
    Serial.println(em::esp_at::ToString(err));
  }

  Serial.println("connect tcp...");
  ret = g_esp_at_manager.Tcpip().ConnectTcp("192.168.8.116", 1234);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("tcp connect failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }
  Serial.println(F("tcp connected"));

  String type;
  String remote_host;
  uint16_t remote_port = 0;
  uint16_t local_port = 0;
  bool client = false;

  ret = g_esp_at_manager.Tcpip().State(&type, &remote_host, &remote_port, &local_port, &client);
  if (ret != em::esp_at::ResultCode::kOK) {
    Serial.print(F("tcp get state failed: "));
    Serial.println(em::esp_at::ToString(ret));
    while (true);
  }

  Serial.print(F("type: "));
  Serial.println(type);
  Serial.print(F("remote_host: "));
  Serial.println(remote_host);
  Serial.print(F("remote_port: "));
  Serial.println(remote_port);
  Serial.print(F("local_port: "));
  Serial.println(local_port);
  Serial.print(F("client: "));
  Serial.println(client);
}

void loop() {
  auto& tcpip = g_esp_at_manager.Tcpip();
  tcpip.GetStream().setTimeout(100);

  auto received_data = tcpip.Receive();
  if (received_data.length > 0) {
    Serial.print(F("received from: "));
    Serial.print(received_data.remote_host);
    Serial.print(':');
    Serial.print(received_data.remote_port);
    Serial.print(F(", length: "));
    Serial.print(received_data.length);

    Serial.print(F(", received data: "));
    while (received_data.length > 0) {
      if (tcpip.GetStream().available() > 0) {
        Serial.print(char(tcpip.GetStream().read()));
        received_data.length--;
      }
    }
    Serial.println();
  }

  static auto s_last_send_time = millis();
  if (millis() - s_last_send_time > 500) {
    String content = String(F("test message with timestamp:")) + s_last_send_time + "\n";
    Serial.print(F("send content: "));
    Serial.println(content);
    tcpip.Send(content.c_str(), content.length());
    s_last_send_time = millis();
  }
}