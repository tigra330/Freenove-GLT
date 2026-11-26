#pragma once
// Einfache Leitungs- und Beschriftungshelfer fuer Anlagenschemata.
// Gerade Leitungen sind Rechtecke (kein Punktarray noetig); Pfeilspitzen nutzen
// statische Punktarrays, die LVGL dauerhaft referenzieren darf.
#include <lvgl.h>

namespace glt {

enum class Dir { Right, Down, Left, Up };

// Horizontale Leitung, y ist die Mittellinie.
lv_obj_t* hline(lv_obj_t* parent, int x, int y, int len, int thick, uint32_t color);
// Vertikale Leitung, x ist die Mittellinie.
lv_obj_t* vline(lv_obj_t* parent, int x, int y, int len, int thick, uint32_t color);
// Offene Pfeilspitze (12 px), Spitze liegt auf (x, y).
lv_obj_t* arrow(lv_obj_t* parent, int x, int y, Dir dir, uint32_t color);

// Kennzeichen / freie Beschriftung. (x, y) = linke obere Ecke.
lv_obj_t* tag(lv_obj_t* parent, const char* text, int x, int y);
// Wertefeld mit Rahmen; Text wird zentriert. Mit setpoint=true als Sollwertfeld.
lv_obj_t* valueBox(lv_obj_t* parent, int x, int y, int w, bool setpoint = false);
// Liefert das Label eines valueBox() (fuer lv_label_set_text).
lv_obj_t* valueBoxLabel(lv_obj_t* box);

}  // namespace glt
