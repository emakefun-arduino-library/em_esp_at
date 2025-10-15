#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
SoftwareSerial g_debug_serial(6, 5);  // RX, TX
em::EspAtManager g_esp_at_manager(Serial);
}  // namespace

void setup() {
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
}

void loop() {
  auto wifi = g_esp_at_manager.Wifi();
  String ip;
  String gateway;
  String netmask;
  em::esp_at::ResultCode result = wifi.Ip(&ip, &gateway, &netmask);
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

  String mac;
  result = wifi.Mac(&mac);
  if (result == em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("mac: "));
    g_debug_serial.println(mac);
  } else {
    g_debug_serial.print(F("wifi mac failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
  }

  String ssid;
  String bssid;
  uint16_t channel;
  int16_t rssi;
  result = wifi.ApInfo(&ssid, &bssid, &channel, &rssi);
  if (result == em::esp_at::ResultCode::kOK) {
    g_debug_serial.print(F("ssid: "));
    g_debug_serial.println(ssid);
    g_debug_serial.print(F("bssid: "));
    g_debug_serial.println(bssid);
    g_debug_serial.print(F("channel: "));
    g_debug_serial.println(channel);
    g_debug_serial.print(F("rssi: "));
    g_debug_serial.println(rssi);
  } else {
    g_debug_serial.print(F("wifi ap info failed: "));
    g_debug_serial.println(em::esp_at::ToString(result));
  }

  delay(1000);
}