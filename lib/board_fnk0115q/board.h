#pragma once
// Boardprofil: Freenove FNK0115Q (5,0" IPS, 800x480, RGB565-Parallel-Panel, GT911-Touch)
// Pins/Timings aus vendor/Sketches/*/lvgl_display.{h,cpp} (FNK0115Q_5_0_IPS).
// Die Anwendungslogik spricht nur diese Schnittstelle an – weitere Displays
// bekommen ein eigenes lib/board_<modell>/ mit derselben API.

#include <lvgl.h>

namespace board {

constexpr int32_t kWidth = 800;
constexpr int32_t kHeight = 480;

const char *name();

// Initialisiert Panel, LVGL-Display, Touch und LVGL-Tick.
// Voraussetzung: lv_init() wurde aufgerufen. Gibt false bei Hardwarefehler zurück.
bool begin();

void setBacklight(bool on);
void setBrightness(uint8_t percent);  // 0..100 (PWM)

}  // namespace board
