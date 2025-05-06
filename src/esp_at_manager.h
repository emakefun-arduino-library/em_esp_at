/**
 * @file esp_at_manager.h
 */
#pragma once

#ifndef _EM_ESP_AT_MANAGER_H_
#define _EM_ESP_AT_MANAGER_H_

#include <Stream.h>
#include <WString.h>

#include "esp_at_mqtt.h"
#include "esp_at_wifi.h"
#include "result_code.h"

namespace em {

class EspAtManager {
 public:
  explicit EspAtManager(Stream& stream) : stream_(stream), wifi_(stream), mqtt_(stream) {
  }

  esp_at::ResultCode Init();

  EspAtWifi& Wifi() {
    return wifi_;
  }

  EspAtMqtt& Mqtt() {
    return mqtt_;
  }

 private:
  EspAtManager(const EspAtManager&) = delete;
  EspAtManager& operator=(const EspAtManager&) = delete;

  esp_at::ResultCode Restart();
  bool CancelSend();

  Stream& stream_;
  EspAtMqtt mqtt_;
  EspAtWifi wifi_;
};
}  // namespace em
#endif