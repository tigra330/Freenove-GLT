#include "gallery_screen.h"

#include "glt.h"
#include "ui_common.h"

namespace ui {
namespace {

using glt::Kind;
using glt::State;

glt::Symbol symbols[12];
State fanState = State::Running;

void changeFan(lv_event_t*) {
  fanState = fanState == State::Running ? State::Off : fanState == State::Off ? State::Fault : State::Running;
  symbols[0].setState(fanState);
  symbols[0].setValue(fanState == State::Running ? "65.0 %" : fanState == State::Fault ? "Stoerung" : "0.0 %");
}

}  // namespace

void GalleryScreen::create() {
  screen_ = makeScreen();
  auto* title = text(screen_, "GLT - Symbolbibliothek  |  DEMO", 16, 10, &lv_font_montserrat_24, 0x263746);
  (void)title;
  const Kind kinds[] = {Kind::Fan, Kind::Fan, Kind::Fan, Kind::Pump, Kind::Heating, Kind::Cooling,
                        Kind::Filter, Kind::Damper, Kind::Valve, Kind::Temperature, Kind::Pressure, Kind::Differential};
  const char* names[] = {"Ventilator EIN", "Ventilator AUS", "Ventilator Fehler", "Pumpe", "Heizregister", "Kuehlregister",
                         "Filter F7", "Luftklappe", "Stellventil", "Temperatur", "Druck", "Differenzdruck"};
  const char* values[] = {"65.0 %", "0.0 %", "Stoerung", "Betrieb", "42.5 %", "0.0 %",
                          "120 Pa", "100 %", "45.0 %", "22.5 C", "300 Pa", "120 Pa"};
  for (int i = 0; i < 12; i++) {
    auto* tile = lv_obj_create(screen_);
    lv_obj_set_pos(tile, 12 + (i % 4) * 197, 50 + (i / 4) * 125);
    lv_obj_set_size(tile, 185, 116);
    lv_obj_set_style_pad_all(tile, 0, 0);
    lv_obj_set_style_radius(tile, 5, 0);
    lv_obj_set_style_bg_color(tile, lv_color_hex(glt::color::Panel), 0);
    lv_obj_set_style_border_color(tile, lv_color_hex(glt::color::PanelBorder), 0);
    lv_obj_set_style_border_width(tile, 1, 0);
    lv_obj_remove_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
    symbols[i].create(tile, kinds[i], (185 - 124) / 2, 5, 52, names[i]);
    symbols[i].setValue(values[i]);
  }
  symbols[0].setState(State::Running);
  symbols[1].setState(State::Off);
  symbols[2].setState(State::Fault);
  symbols[3].setState(State::Running);
  symbols[7].setState(State::Running);
  symbols[8].setState(State::Running);
  lv_obj_add_event_cb(symbols[0].object(), changeFan, LV_EVENT_CLICKED, nullptr);

  auto* f = footer(screen_, ScreenId::Gallery);
  text(f, "Ersten Ventilator antippen: EIN / AUS / Stoerung", 290, 13, &lv_font_montserrat_14, 0x344454);
}

}  // namespace ui
