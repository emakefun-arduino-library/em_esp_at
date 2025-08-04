#include "SoftwareSerial.h"
#include "esp_at_manager.h"

#define WIFI_SSID F("emakefun")
#define WIFI_PASSWD F("501416wf")

namespace {
SoftwareSerial g_at_serial(5, 6);  // RX, TX
em::EspAtManager g_esp_at_manager(g_at_serial);
}  // namespace

void setup() {
  Serial.begin(115200);
  g_at_serial.begin(9600);

  auto err = em::esp_at::ResultCode::kOK;

  Serial.println("module init");
  err = g_esp_at_manager.Init();
  if (err != em::esp_at::ResultCode::kOK) {
    Serial.print("module init failed: ");
    Serial.println(em::esp_at::ToString(err));
    while (true);
  }

  auto wifi = g_esp_at_manager.Wifi();
  Serial.println("wifi connecting...");
  err = wifi.ConnectWifi(WIFI_SSID, WIFI_PASSWD);
  if (err != em::esp_at::ResultCode::kOK) {
    Serial.print("wifi connect failed: ");
    Serial.println(em::esp_at::ToString(err));
    while (true);
  }

  Serial.println("wifi connected");
}

void loop() {
  auto wifi = g_esp_at_manager.Wifi();
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

  String mac;
  err = wifi.Mac(&mac);
  if (err == em::esp_at::ResultCode::kOK) {
    Serial.print("mac: ");
    Serial.println(mac);
  } else {
    Serial.print("wifi mac failed: ");
    Serial.println(em::esp_at::ToString(err));
  }

  String ssid;
  String bssid;
  uint16_t channel;
  int16_t rssi;
  err = wifi.ApInfo(&ssid, &bssid, &channel, &rssi);
  if (err == em::esp_at::ResultCode::kOK) {
    Serial.print("ssid: ");
    Serial.println(ssid);
    Serial.print("bssid: ");
    Serial.println(bssid);
    Serial.print("channel: ");
    Serial.println(channel);
    Serial.print("rssi: ");
    Serial.println(rssi);
  } else {
    Serial.print("wifi ap info failed: ");
    Serial.println(em::esp_at::ToString(err));
  }

  delay(1000);
}