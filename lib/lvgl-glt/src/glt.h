#pragma once
#include <lvgl.h>

#include "glt_colors.h"
#include "glt_lines.h"

namespace glt {

// Neue Werte nur anhaengen (Rueckwaertskompatibilitaet).
enum class Kind { Fan, Pump, Heating, Cooling, Filter, Damper, Valve, Temperature, Pressure, Differential, Compressor, Box };
// Normal = Messstelle ohne Befund (weisser Koerper). Fault/Unknown zeigen
// zusaetzlich ein Textzeichen ("!" / "?"), nicht nur eine Farbe.
enum class State { Off, Running, Fault, Unknown, Normal };
// Richtung des Symbols (Ventilator-/Pumpendreieck). Default je Kind.
enum class Orientation { Default, Right, Down, Left, Up };

class Symbol {
 public:
  // Kachel-Variante (Version 0.1): Root size+72 x size+52 mit Name und Wert.
  void create(lv_obj_t* parent, Kind kind, int x, int y, int size, const char* name);
  // Kompakte Variante fuer Anlagenschemata: nur der Symbolkoerper, zentriert auf
  // (cx, cy). Beschriftungen werden frei mit addTag()/addValue() platziert.
  void createCompact(lv_obj_t* parent, Kind kind, int cx, int cy, int size,
                     Orientation orientation = Orientation::Default);

  void setState(State state);
  void setValue(const char* value);
  // Innentext (z. B. "F7" am Filter, "+"/"-" am Register, "FU" an Box).
  void setMark(const char* mark);

  // Freie Beschriftung relativ zum Symbolkoerper (Geschwister im Parent).
  lv_obj_t* addTag(const char* text, lv_align_t align, int dx = 0, int dy = 0);
  // Wertefeld relativ zum Symbolkoerper; wird Ziel von setValue().
  lv_obj_t* addValue(lv_align_t align, int dx, int dy, int width);

  lv_obj_t* object() const { return root_; }
  lv_obj_t* body() const { return body_; }

 private:
  void build(lv_obj_t* parent, Kind kind, int x, int y, int size, bool compact, Orientation o);
  void updateBadge(State state);

  lv_obj_t *root_ = nullptr, *body_ = nullptr, *value_ = nullptr, *mark_ = nullptr, *badge_ = nullptr;
  Kind kind_ = Kind::Fan;
  bool compact_ = false;
};

}  // namespace glt
