#include "detail_dialog.h"

#include <stdio.h>

#include "glt.h"
#include "ui_common.h"

namespace ui {
namespace {

constexpr int kW = 380, kH = 300;

void closeCb(lv_event_t* e) {
  // Nur Klicks direkt auf den abgedunkelten Hintergrund oder den Button schliessen.
  auto* self = static_cast<DetailDialog*>(lv_event_get_user_data(e));
  if (lv_event_get_target(e) == lv_event_get_current_target(e)) self->close();
}

}  // namespace

void DetailDialog::create() {
  overlay_ = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(overlay_);
  lv_obj_set_size(overlay_, 800, 480);
  lv_obj_set_style_bg_color(overlay_, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(overlay_, LV_OPA_40, 0);
  lv_obj_add_flag(overlay_, LV_OBJ_FLAG_CLICKABLE);  // blockiert Touch auf das Schema
  lv_obj_remove_flag(overlay_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(overlay_, closeCb, LV_EVENT_CLICKED, this);

  auto* box = panel(overlay_, (800 - kW) / 2, (480 - kH) / 2, kW, kH, glt::color::Panel, 1);
  lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);  // Klick im Fenster schliesst nicht
  lv_obj_set_style_radius(box, 6, 0);

  auto* head = panel(box, 1, 1, kW - 2, 40, 0xdfe6ec);
  title_ = text(head, "", 12, 10, &lv_font_montserrat_16, glt::color::Ink);

  static const char* labels[RowCount] = {"Typ", "Istwert", "Sollwert", "Status", "Datenqualitaet", "Quelle"};
  for (int i = 0; i < RowCount; i++) {
    int y = 50 + i * 28;
    text(box, labels[i], 16, y, &lv_font_montserrat_16, glt::color::InkMuted);
    rows_[i] = text(box, "---", 150, y, &lv_font_montserrat_16, glt::color::Ink);
  }
  text(box, "Nur Anzeige - keine Schreibbefehle (DEMO)", 16, kH - 66, &lv_font_montserrat_14, glt::color::Demo);
  auto* b = button(box, "Schliessen", kW - 124, kH - 52, 112, 40, closeCb, this);
  (void)b;
  lv_obj_add_flag(overlay_, LV_OBJ_FLAG_HIDDEN);
}

void DetailDialog::open(model::Pt p, const model::Snapshot& s) {
  point_ = p;
  open_ = true;
  const auto& i = model::info(p);
  lv_label_set_text_fmt(title_, "%s  -  %s", i.id, i.name);
  update(s);
  lv_obj_remove_flag(overlay_, LV_OBJ_FLAG_HIDDEN);
}

void DetailDialog::close() {
  open_ = false;
  if (overlay_) lv_obj_add_flag(overlay_, LV_OBJ_FLAG_HIDDEN);
}

void DetailDialog::update(const model::Snapshot& s) {
  if (!open_) return;
  const auto& i = model::info(point_);
  const auto& d = s[point_];
  char buf[48];
  lv_label_set_text(rows_[Type], model::kindText(i.kind));
  if (i.hasValue) {
    model::formatValue(buf, sizeof buf, point_, model::usable(d) ? d.actual : model::Value{});
    lv_label_set_text(rows_[Actual], buf);
  } else {
    lv_label_set_text(rows_[Actual], "- (kein Analogwert)");
  }
  if (i.hasSetpoint) {
    model::formatValue(buf, sizeof buf, point_, model::usable(d) ? d.setpoint : model::Value{});
    lv_label_set_text(rows_[Setpoint], buf);
  } else {
    lv_label_set_text(rows_[Setpoint], "-");
  }
  if (point_ == model::Pt::HSB)
    lv_label_set_text_fmt(rows_[Status], "%s / %s", model::modeText(s.mode), model::statusText(point_, d.status));
  else
    lv_label_set_text(rows_[Status], model::statusText(point_, d.status));
  lv_label_set_text(rows_[Quality], model::qualityText(d.quality));
  lv_obj_set_style_text_color(rows_[Quality],
                              lv_color_hex(d.quality == model::Quality::Good ? glt::color::Ink : glt::color::StateFault), 0);
  lv_label_set_text(rows_[Source], s.demo ? "Demo (synthetisch)" : "Datenquelle");
}

}  // namespace ui
