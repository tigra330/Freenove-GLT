#pragma once
// Modales Detailfenster fuer einen Datenpunkt (nur Anzeige, keine Schreibbefehle).
// Liegt auf lv_layer_top() und wird einmalig erzeugt; open()/close() schalten
// nur die Sichtbarkeit, update() aktualisiert die Werte des offenen Punkts.
#include <lvgl.h>

#include "../model/rlt_data.h"

namespace ui {

class DetailDialog {
 public:
  void create();
  void open(model::Pt p, const model::Snapshot& s);
  void close();
  void update(const model::Snapshot& s);
  bool isOpen() const { return open_; }

 private:
  enum Row { Type, Actual, Setpoint, Status, Quality, Source, RowCount };
  lv_obj_t* overlay_ = nullptr;
  lv_obj_t* title_ = nullptr;
  lv_obj_t* rows_[RowCount] = {};
  model::Pt point_ = model::Pt::HSB;
  bool open_ = false;
};

}  // namespace ui
