#pragma once
#include <lvgl.h>

namespace ui {

enum class ScreenId { Rlt, Gallery };

// Implementiert in app.cpp. Nur im LVGL-Kontext aufrufen.
void navigate(ScreenId id);

constexpr int kHeaderH = 36;
constexpr int kFooterY = 436;
constexpr int kFooterH = 44;

// Leerer Screen mit GLT-Hintergrund, ohne Scrollen.
lv_obj_t* makeScreen();
// Flaches Rechteck ohne Theme-Stil (nicht klickbar).
lv_obj_t* panel(lv_obj_t* parent, int x, int y, int w, int h, uint32_t bg, int border = 0);
// Textlabel mit Font und Farbe.
lv_obj_t* text(lv_obj_t* parent, const char* t, int x, int y, const lv_font_t* font, uint32_t color);
// Schaltflaeche mit zentriertem Text; liefert den Button.
lv_obj_t* button(lv_obj_t* parent, const char* t, int x, int y, int w, int h, lv_event_cb_t cb,
                 void* user = nullptr);
// Fussleiste mit Navigation; active markiert den aktuellen Screen.
lv_obj_t* footer(lv_obj_t* screen, ScreenId active);

}  // namespace ui
