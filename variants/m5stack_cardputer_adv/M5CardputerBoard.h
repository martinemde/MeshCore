#pragma once

#include <helpers/ESP32Board.h>
#include <Arduino.h>
#include <Wire.h>
#include <M5Cardputer.h>

class M5CardputerADVBoard : public ESP32Board {
public:
  M5CardputerADVBoard() : ESP32Board() {
  }

  void begin() {
    // Initialize the external I2C bus before M5Cardputer starts its internal bus.
    ESP32Board::begin();

    // Step 3: Initialize M5Cardputer hardware with keyboard enabled
    auto cfg = M5.config();
    cfg.clear_display = true;
    cfg.internal_imu = false;
    cfg.internal_rtc = true;
    cfg.internal_spk = true;
    cfg.internal_mic = true;
    M5Cardputer.begin(cfg, true);
    delay(100);
    M5Cardputer.Keyboard.begin();

    // Cap LoRa-1262 antenna switch: PI4IOE P0 HIGH on internal I2C (8/9).
    bool cap_ok = M5.In_I2C.writeRegister8(0x43, 0x03, 0x01, 100000);
    cap_ok = cap_ok && M5.In_I2C.writeRegister8(0x43, 0x07, 0x00, 100000);
    cap_ok = cap_ok && M5.In_I2C.writeRegister8(0x43, 0x05, 0x01, 100000);
    Serial.println(cap_ok ? "LoRa Cap antenna switch enabled" : "ERROR: LoRa Cap antenna switch init failed");

    Serial.println("M5Stack Cardputer-Adv initialized");
    Serial.print("Battery voltage: ");
    Serial.print(getBattMilliVolts());
    Serial.println(" mV");
  }

  bool startOTAUpdate(const char*, char[]) override {
    return false; // DualMesh owns both OTA slots.
  }

  uint16_t getBattMilliVolts() override {
    // Battery voltage on GPIO 10 with 2:1 voltage divider
    uint32_t sum = 0;
    for (int i = 0; i < 8; i++) {
      sum += analogReadMilliVolts(10);
      delay(5);
    }
    return (sum / 8) * 2;  // Apply 2:1 divider correction
  }

  const char* getManufacturerName() const override {
    return "M5Stack Cardputer ADV";
  }
};
