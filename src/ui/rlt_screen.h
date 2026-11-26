#pragma once
// RLT-Anlagenschema (Startansicht). Objekte werden einmal erzeugt; update()
// setzt nur Zustaende und Werte aus dem Snapshot.
#include <lvgl.h>

#include "../model/rlt_data.h"
#include "glt.h"

namespace ui {

class RltScreen {
 public:
  using PointHandler = void (*)(model::Pt);
  using ActionHandler = void (*)();

  void create(PointHandler onPoint, ActionHandler onDemo);
  void update(const model::Snapshot& s);
  void setDemoText(const char* t);
  lv_obj_t* screen() const { return screen_; }

 private:
  struct Item {
    model::Pt pt;
    glt::Symbol sym;
    lv_obj_t* setpoint = nullptr;  // optionales Sollwertfeld
    bool hasSymbol = false;
  };
  static constexpr int kMaxItems = 40;

  Item& add(model::Pt pt);
  Item& symbol(model::Pt pt, glt::Kind kind, int cx, int cy, int size,
               glt::Orientation o = glt::Orientation::Default);
  void sensorAbove(model::Pt pt, glt::Kind kind, int x, int ductY, int size);
  void sensorBelow(model::Pt pt, glt::Kind kind, int x, int ductY, int size);
  void bracket(int x1, int x2, int top, int ductY);
  void clickable(lv_obj_t* o, model::Pt pt);

  void buildHeader();
  void buildDucts();
  void buildExhaust();
  void buildSupply();
  void buildFooter(ActionHandler onDemo);

  lv_obj_t* screen_ = nullptr;
  lv_obj_t* hsbLabel_ = nullptr;
  lv_obj_t* hsbBox_ = nullptr;
  lv_obj_t* bmzLabel_ = nullptr;
  lv_obj_t* bmzBox_ = nullptr;
  lv_obj_t* demoLabel_ = nullptr;
  Item items_[kMaxItems];
  int count_ = 0;
  PointHandler onPoint_ = nullptr;
};

}  // namespace ui
