#include "board.h"

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <esp_heap_caps.h>

namespace {

// ---- Pins (Freenove FNK0115Q) ----
constexpr int PIN_BL = 2;

constexpr int PIN_DE = 40, PIN_VSYNC = 41, PIN_HSYNC = 39, PIN_PCLK = 42;
constexpr int PIN_R0 = 45, PIN_R1 = 48, PIN_R2 = 47, PIN_R3 = 21, PIN_R4 = 14;
constexpr int PIN_G0 = 5, PIN_G1 = 6, PIN_G2 = 7, PIN_G3 = 15, PIN_G4 = 16, PIN_G5 = 4;
constexpr int PIN_B0 = 8, PIN_B1 = 3, PIN_B2 = 46, PIN_B3 = 9, PIN_B4 = 1;

// GT911 – GPIO19/20 sind die nativen USB-Pins des S3, deshalb laeuft
// die serielle Konsole ueber den CH340 (UART0), nicht ueber USB-CDC.
constexpr int TOUCH_SDA = 19, TOUCH_SCL = 20, TOUCH_INT = 18, TOUCH_RST = 38;

// Rohkoordinaten des GT911 sind gespiegelt (Herstellerwerte: X 800->0, Y 480->0)
constexpr int TOUCH_MAP_X1 = 800, TOUCH_MAP_X2 = 0;
constexpr int TOUCH_MAP_Y1 = 480, TOUCH_MAP_Y2 = 0;

// ---- Panel (IPS-Timings laut Hersteller) ----
Arduino_ESP32RGBPanel *rgbPanel = new Arduino_ESP32RGBPanel(
    PIN_DE, PIN_VSYNC, PIN_HSYNC, PIN_PCLK,
    PIN_R0, PIN_R1, PIN_R2, PIN_R3, PIN_R4,
    PIN_G0, PIN_G1, PIN_G2, PIN_G3, PIN_G4, PIN_G5,
    PIN_B0, PIN_B1, PIN_B2, PIN_B3, PIN_B4,
    0 /* hsync_polarity */, 4 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 8 /* hsync_back_porch */,
    0 /* vsync_polarity */, 4 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 8 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 13000000 /* prefer_speed */,
    false /* useBigEndian */, 0 /* de_idle_high */, 0 /* pclk_idle_high */,
    // Bounce-Buffer: Das Panel liest sonst per DMA direkt aus dem PSRAM. Bei
    // WLAN-Last/PSRAM-Zugriffen reicht die Bandbreite kurz nicht -> Bild "zuckt"
    // bzw. verschiebt sich. Mit 2x10 Zeilen im internen RAM ist das entkoppelt.
    // (Muss die Framebuffer-Größe glatt teilen: 800*480 / 8000 = 48.)
    board::kWidth * 10 /* bounce_buffer_size_px */);

Arduino_RGB_Display *gfx =
    new Arduino_RGB_Display(board::kWidth, board::kHeight, rgbPanel, 0 /* rotation */, true /* auto_flush */);

TAMC_GT911 touch(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, board::kWidth, board::kHeight);

// 1/10 Bildschirm als LVGL-Zeichenpuffer (interner RAM = schnell, Fallback PSRAM)
constexpr size_t kBufLines = board::kHeight / 10;
constexpr size_t kBufBytes = board::kWidth * kBufLines * sizeof(uint16_t);

void flushCb(lv_display_t *disp, const lv_area_t *area, uint8_t *pxMap) {
  const int32_t w = lv_area_get_width(area);
  const int32_t h = lv_area_get_height(area);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<uint16_t *>(pxMap), w, h);
  lv_display_flush_ready(disp);
}

// ---- GT911 direkt auslesen
// TAMC_GT911::read() wertet die Punktanzahl auch aus, wenn das "Daten bereit"-Bit
// (0x814E Bit 7) nicht gesetzt ist. Bei LVGL-Abfragen alle 16 ms meldet es dadurch
// mitten im Tipp kurz "nicht berührt" -> LVGL sieht Loslassen + Drücken = zweiter
// Klick (Schalter geht an und sofort wieder aus). Hier: nur frische Daten
// übernehmen, sonst letzten Zustand halten; Loslassen zusätzlich entprellen.
constexpr uint8_t kGtAddr = 0x5D;  // Adresse nach Reset durch TAMC_GT911::begin()
constexpr uint16_t kGtPointInfo = 0x814E;
constexpr uint16_t kGtPoint1 = 0x814F;
constexpr uint32_t kReleaseDebounceMs = 40;  // so lange "keine Berührung" bevor losgelassen gilt
constexpr uint32_t kStaleTimeoutMs = 250;    // keine Daten mehr während gedrückt -> loslassen

struct {
  bool down = false;       // letzter gemeldeter Rohzustand des Controllers
  int32_t x = 0, y = 0;
  uint32_t lastDown = 0;   // letzter Bericht mit Berührung
  uint32_t lastReport = 0; // letzter Bericht überhaupt
} gTouch;

bool gtRead(uint16_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(kGtAddr);
  Wire.write(reg >> 8);
  Wire.write(reg & 0xFF);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(kGtAddr, len) != len) return false;
  for (uint8_t i = 0; i < len; ++i) buf[i] = Wire.read();
  return true;
}

void gtWrite(uint16_t reg, uint8_t value) {
  Wire.beginTransmission(kGtAddr);
  Wire.write(reg >> 8);
  Wire.write(reg & 0xFF);
  Wire.write(value);
  Wire.endTransmission();
}

void touchReadCb(lv_indev_t *, lv_indev_data_t *data) {
  const uint32_t now = millis();
  uint8_t info = 0;
  if (gtRead(kGtPointInfo, &info, 1) && (info & 0x80)) {  // neue Daten vorhanden
    const uint8_t n = info & 0x0F;
    uint8_t d[7];
    if (n >= 1 && n <= 5 && gtRead(kGtPoint1, d, sizeof(d))) {
      const int32_t rawX = d[1] | (d[2] << 8);
      const int32_t rawY = d[3] | (d[4] << 8);
      // Wie bisher: TAMC spiegelte (ROTATION_NORMAL), danach map() 800->0 – netto Rohwert
      const int32_t x = map(board::kWidth - rawX, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, board::kWidth - 1);
      const int32_t y = map(board::kHeight - rawY, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, board::kHeight - 1);
      gTouch.x = constrain(x, 0, board::kWidth - 1);
      gTouch.y = constrain(y, 0, board::kHeight - 1);
      gTouch.down = true;
      gTouch.lastDown = now;
    } else {
      gTouch.down = false;
    }
    gTouch.lastReport = now;
    gtWrite(kGtPointInfo, 0);  // Bericht quittieren
  } else if (gTouch.down && now - gTouch.lastReport > kStaleTimeoutMs) {
    gTouch.down = false;  // Controller meldet nichts mehr -> nicht ewig "gedrückt"
  }

  const bool pressed = gTouch.down || (now - gTouch.lastDown < kReleaseDebounceMs);
  data->point.x = gTouch.x;
  data->point.y = gTouch.y;
  data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

uint32_t tickCb() { return millis(); }

}  // namespace

namespace board {

const char *name() { return "freenove_fnk0115q_5in_ips"; }

void setBrightness(uint8_t percent) {
  static bool attached = false;
  if (!attached) attached = ledcAttach(PIN_BL, 20000 /* Hz */, 8 /* bit */);
  percent = percent > 100 ? 100 : percent;
  ledcWrite(PIN_BL, (uint32_t)percent * 255 / 100);
}

void setBacklight(bool on) { setBrightness(on ? 100 : 0); }

bool begin() {
  setBrightness(0);

  if (!psramFound()) {
    Serial.println("[board] FEHLER: kein PSRAM gefunden – Framebuffer (750 KB) passt nicht in den internen RAM.");
    return false;
  }

  if (!gfx->begin()) {
    Serial.println("[board] FEHLER: RGB-Panel-Init fehlgeschlagen");
    return false;
  }
  gfx->fillScreen(RGB565_BLACK);

  void *buf = heap_caps_malloc(kBufBytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!buf) buf = heap_caps_malloc(kBufBytes, MALLOC_CAP_SPIRAM);
  if (!buf) {
    Serial.println("[board] FEHLER: kein Speicher fuer LVGL-Puffer");
    return false;
  }

  lv_tick_set_cb(tickCb);

  lv_display_t *disp = lv_display_create(kWidth, kHeight);
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(disp, flushCb);
  lv_display_set_buffers(disp, buf, nullptr, kBufBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);

  Wire.setBufferSize(256);  // GT911 liest beim Start 185 Byte Konfiguration
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  touch.begin();
  touch.setRotation(ROTATION_NORMAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchReadCb);

  return true;
}

}  // namespace board
