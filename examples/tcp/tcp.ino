#include <Arduino.h>

#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
constexpr uint8_t kLedPin = 13;

constexpr uint16_t kTcpServerPort = 1234;
const String kTcpServerHost = "192.168.8.116";

SoftwareSerial g_debug_serial(6, 5);  // RX, TX
em::EspAtManager g_esp_at_manager(Serial);
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
  g_debug_serial.println(F("wifi connecting..."));
  result = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("wifi connect failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }

  g_debug_serial.println(F("wifi connected"));
  String ip;
  String gateway;
  String netmask;
  result = wifi.Ip(&ip, &gateway, &netmask);
  if (result == em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("ip: "));
    g_debug_serial.println(ip);
    g_debug_serial.print(F("gateway: "));
    g_debug_serial.println(gateway);
    g_debug_serial.print(F("netmask: "));
    g_debug_serial.println(netmask);
  } else {
    g_debug_serial.print(F("wifi ip failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
  }

  g_debug_serial.println(F("connect tcp..."));
  result = g_esp_at_manager.Tcpip().ConnectTcp(kTcpServerHost, kTcpServerPort);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("tcp connect failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }
  g_debug_serial.println(F("tcp connected"));

  String type;
  String remote_host;
  uint16_t remote_port = 0;
  uint16_t local_port = 0;
  bool client = false;

  result = g_esp_at_manager.Tcpip().State(&type, &remote_host, &remote_port, &local_port, &client);
  if (result != em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("tcp get state failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
    while (true);
  }

  g_debug_serial.print(F("type: "));
  g_debug_serial.println(type);
  g_debug_serial.print(F("remote_host: "));
  g_debug_serial.println(remote_host);
  g_debug_serial.print(F("remote_port: "));
  g_debug_serial.println(remote_port);
  g_debug_serial.print(F("local_port: "));
  g_debug_serial.println(local_port);
  g_debug_serial.print(F("client: "));
  g_debug_serial.println(client);
}

void loop() {
  auto& tcpip = g_esp_at_manager.Tcpip();
  tcpip.GetStream().setTimeout(100);

  auto received_data = tcpip.Receive();
  if (received_data.length > 0) {
    g_debug_serial.print(F("received from: "));
    g_debug_serial.print(received_data.remote_host);
    g_debug_serial.print(':');
    g_debug_serial.print(received_data.remote_port);
    g_debug_serial.print(F(", length: "));
    g_debug_serial.print(received_data.length);

    g_debug_serial.print(F(", received data: "));
    uint16_t remaining_length = received_data.length;
    String content = "";
    while (remaining_length > 0) {
      if (tcpip.GetStream().available() > 0) {
        content += char(tcpip.GetStream().read());
        remaining_length--;
      }
    }
    g_debug_serial.println(content);

    if (received_data.remote_host == kTcpServerHost && received_data.remote_port == kTcpServerPort && content.length() == received_data.length) {
      if (content == "led on") {
        digitalWrite(kLedPin, HIGH);
        g_debug_serial.println(F("LED ON"));
      } else if (content == "led off") {
        digitalWrite(kLedPin, LOW);
        g_debug_serial.println(F("LED OFF"));
      }
    }
  }

  static auto s_last_send_time = millis();
  if (millis() - s_last_send_time > 500) {
    String content = (digitalRead(kLedPin) == HIGH) ? "led off" : "led on";
    g_debug_serial.print(F("send content: "));
    g_debug_serial.println(content);
    tcpip.Send(content.c_str(), content.length());
    s_last_send_time = millis();
  }
}