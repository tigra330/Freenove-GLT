#include "rlt_screen.h"

#include <stdint.h>

#include "ui_common.h"

namespace ui {
namespace {

using glt::Dir;
using glt::Kind;
using glt::Orientation;
using model::Pt;
namespace c = glt::color;

// Kanalachsen und Geometrie (Bildschirm 800 x 480)
constexpr int EX = 120;     // Abluft/Fortluft
constexpr int SU = 290;     // Aussenluft/Zuluft
constexpr int DUCT = 10;    // Kanalbreite
constexpr int X0 = 22;      // Kanalanfang links
constexpr int XR = 760;     // Steigleitung zum Raum
constexpr int ROOM_X = 700, ROOM_Y = 145, ROOM_W = 92, ROOM_H = 110;
constexpr int PIPE_END = 420;
constexpr int COIL = 44;    // Registergroesse
constexpr int SENSOR = 20;
constexpr int STALK = 6;

RltScreen::PointHandler g_onPoint = nullptr;
RltScreen::ActionHandler g_onDemo = nullptr;
constexpr int kMaxDecor = 10;
glt::Symbol g_decor[kMaxDecor];  // Register/Filter ohne eigenen Datenpunkt
int g_decorCount = 0;

void pointCb(lv_event_t* e) {
  if (g_onPoint) g_onPoint(static_cast<Pt>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e))));
}
void demoCb(lv_event_t*) {
  if (g_onDemo) g_onDemo();
}

glt::Symbol& decor(lv_obj_t* parent, Kind kind, int cx, int cy, const char* mark) {
  LV_ASSERT(g_decorCount < kMaxDecor);
  glt::Symbol& s = g_decor[g_decorCount++];
  s.createCompact(parent, kind, cx, cy, COIL);
  s.setMark(mark);
  lv_obj_remove_flag(s.object(), LV_OBJ_FLAG_CLICKABLE);
  return s;
}

void chevron(lv_obj_t* parent, const char* sym, int x, int y) {
  auto* l = text(parent, sym, x, y - 8, &lv_font_montserrat_14, c::Ink);
  (void)l;
}

// Pipe mit Pfeil am unteren Ende: up=true -> Vorlauf kommt von unten.
void pipeDown(lv_obj_t* p, int x, int yTop, uint32_t color, bool up) {
  glt::vline(p, x, yTop, PIPE_END - yTop, 2, color);
  if (up)
    glt::arrow(p, x, PIPE_END - 10, Dir::Up, c::Ink);
  else
    glt::arrow(p, x, PIPE_END + 8, Dir::Down, c::Ink);
}

glt::State toState(Pt p, const model::DataPoint& d) {
  using model::PointKind;
  using model::Quality;
  using model::Status;
  if (d.quality != Quality::Good) return glt::State::Unknown;  // unsicher/Komm.-fehler/unbekannt
  if (d.status == Status::Fault) return glt::State::Fault;
  if (d.status == Status::Unknown) return glt::State::Unknown;
  switch (model::info(p).kind) {
    case PointKind::Temperature:
    case PointKind::Pressure:
      return glt::State::Normal;
    case PointKind::Contact:
      return d.status == Status::On ? glt::State::Running : glt::State::Normal;
    case PointKind::DiffPressure:
      if (model::info(p).hasValue) return glt::State::Normal;
      return d.status == Status::On ? glt::State::Running : glt::State::Normal;
    default:
      return d.status == Status::On ? glt::State::Running : glt::State::Off;
  }
}

}  // namespace

RltScreen::Item& RltScreen::add(Pt pt) {
  LV_ASSERT(count_ < kMaxItems);
  Item& it = items_[count_++];
  it.pt = pt;
  return it;
}

void RltScreen::clickable(lv_obj_t* o, Pt pt) {
  lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, pointCb, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<intptr_t>(pt)));
}

RltScreen::Item& RltScreen::symbol(Pt pt, Kind kind, int cx, int cy, int size, Orientation o) {
  Item& it = add(pt);
  it.sym.createCompact(screen_, kind, cx, cy, size, o);
  it.hasSymbol = true;
  clickable(it.sym.object(), pt);
  return it;
}

void RltScreen::sensorAbove(Pt pt, Kind kind, int x, int ductY, int size) {
  int bottom = ductY - DUCT / 2;
  glt::vline(screen_, x, bottom - STALK, STALK, 2, c::Ink);
  symbol(pt, kind, x, bottom - STALK - size / 2, size);
}

void RltScreen::sensorBelow(Pt pt, Kind kind, int x, int ductY, int size) {
  int top = ductY + DUCT / 2;
  glt::vline(screen_, x, top, STALK, 2, c::Ink);
  symbol(pt, kind, x, top + STALK + size / 2, size);
}

// Differenzdruck-Abgriff: Leitungen von beiden Seiten der Komponente zur Messdose.
void RltScreen::bracket(int x1, int x2, int top, int ductY) {
  glt::hline(screen_, x1, top, x2 - x1, 2, c::Ink);
  glt::vline(screen_, x1, top, ductY - top, 2, c::Ink);
  glt::vline(screen_, x2, top, ductY - top, 2, c::Ink);
}

void RltScreen::create(PointHandler onPoint, ActionHandler onDemo) {
  g_onPoint = onPoint;
  g_onDemo = onDemo;
  screen_ = makeScreen();
  buildDucts();     // Hintergrund: Raum, Kanaele, Rohre
  buildExhaust();   // danach Komponenten, Sensoren und Werte
  buildSupply();
  buildHeader();
  buildFooter(onDemo);
}

void RltScreen::buildHeader() {
  auto* h = panel(screen_, 0, 0, 800, kHeaderH, c::Panel);
  lv_obj_set_style_border_width(h, 1, 0);
  lv_obj_set_style_border_side(h, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_set_style_border_color(h, lv_color_hex(c::PanelBorder), 0);
  text(h, "RLT  Innenzone BT C", 10, 9, &lv_font_montserrat_16, c::Ink);

  auto* demo = panel(h, 196, 6, 206, 24, c::Demo);
  lv_obj_set_style_radius(demo, 4, 0);
  auto* dl = text(demo, "DEMO - synthetische Werte", 0, 0, &lv_font_montserrat_14, 0xffffff);
  lv_obj_center(dl);

  hsbBox_ = panel(h, 462, 4, 150, 28, c::Panel, 1);
  hsbLabel_ = text(hsbBox_, "HSB  ---", 0, 0, &lv_font_montserrat_14, c::Ink);
  lv_obj_center(hsbLabel_);
  clickable(hsbBox_, Pt::HSB);

  bmzBox_ = panel(h, 618, 4, 176, 28, c::Panel, 1);
  bmzLabel_ = text(bmzBox_, "BMZ  ---", 0, 0, &lv_font_montserrat_14, c::Ink);
  lv_obj_center(bmzLabel_);
  clickable(bmzBox_, Pt::BMZ);
}

void RltScreen::buildFooter(ActionHandler) {
  auto* f = footer(screen_, ScreenId::Rlt);
  auto* b = button(f, "Demo: ---", 276, 5, 196, 34, demoCb);
  lv_obj_set_style_bg_color(b, lv_color_hex(c::Demo), 0);
  demoLabel_ = lv_obj_get_child(b, 0);
  text(f, "Cyan = Sollwert    ! = Stoerung", 484, 4, &lv_font_montserrat_14, c::Ink);
  text(f, "? = unbekannt / Komm.-fehler", 484, 22, &lv_font_montserrat_14, c::Ink);
}

void RltScreen::setDemoText(const char* t) {
  if (demoLabel_) lv_label_set_text_fmt(demoLabel_, "Demo: %s", t);
}

void RltScreen::buildDucts() {
  auto* s = screen_;
  // Raum
  auto* room = panel(s, ROOM_X, ROOM_Y, ROOM_W, ROOM_H, 0xf6f8fa, 1);
  auto* rl = text(room, "Innenzone\nBT C", 0, 36, &lv_font_montserrat_14, c::Ink);
  lv_obj_set_width(rl, ROOM_W - 2);
  lv_obj_set_style_text_align(rl, LV_TEXT_ALIGN_CENTER, 0);
  glt::arrow(s, XR, ROOM_Y + 3, Dir::Up, c::Ink);
  glt::tag(s, "AB", XR - 10, ROOM_Y + 17);
  glt::tag(s, "ZL", XR - 9, ROOM_Y + ROOM_H - 36);
  glt::arrow(s, XR, ROOM_Y + ROOM_H - 17, Dir::Up, c::Ink);

  // Abluft / Fortluft (rechts -> links)
  glt::hline(s, X0, EX, XR + DUCT / 2 - X0, DUCT, c::AirExhaust);
  glt::vline(s, XR, EX - DUCT / 2, ROOM_Y - (EX - DUCT / 2), DUCT, c::AirExhaust);
  glt::arrow(s, 6, EX, Dir::Left, c::Ink);
  glt::tag(s, "FO", 4, EX - 30);
  chevron(s, LV_SYMBOL_LEFT, 515, EX);
  chevron(s, LV_SYMBOL_LEFT, 690, EX);

  // Aussenluft / Zuluft (links -> rechts)
  glt::hline(s, X0, SU, 320 - X0, DUCT, c::AirOutdoor);
  glt::hline(s, 320, SU, 375 - 320, DUCT, c::AirPreheated);
  glt::hline(s, 375, SU, XR + DUCT / 2 - 375, DUCT, c::AirSupply);
  glt::vline(s, XR, ROOM_Y + ROOM_H, SU + DUCT / 2 - (ROOM_Y + ROOM_H), DUCT, c::AirSupply);
  glt::arrow(s, 20, SU, Dir::Right, c::Ink);
  glt::tag(s, "AU", 4, SU - 30);
  chevron(s, LV_SYMBOL_RIGHT, 125, SU);
  chevron(s, LV_SYMBOL_RIGHT, 443, SU);
  chevron(s, LV_SYMBOL_RIGHT, 722, SU);

  // WRG-Kreis zwischen den Registern (blau links, rot rechts)
  glt::vline(s, 310, EX + COIL / 2, SU - EX - COIL, 2, c::PipeHeatReturn);
  glt::vline(s, 330, EX + COIL / 2, SU - EX - COIL, 2, c::PipeHeatFlow);
  glt::hline(s, 310, 222, 20, 2, c::PipeHeatReturn);  // Beimischung ueber Y04

  // Register-Anschluesse nach unten
  pipeDown(s, 367, SU + COIL / 2, c::PipeHeatFlow, true);     // Heizregister
  pipeDown(s, 383, SU + COIL / 2, c::PipeHeatReturn, false);
  pipeDown(s, 492, SU + COIL / 2, c::PipeColdFlow, true);     // Kuehlregister
  pipeDown(s, 508, SU + COIL / 2, c::PipeColdReturn, false);
  //pipeDown(s, 692, SU + COIL / 2, c::PipeHeatFlow, true);     // Nachheizregister
  //pipeDown(s, 708, SU + COIL / 2, c::PipeHeatReturn, false);

  // Zusatzregister Abluft -> "zum Verdichter !"
  /*
  glt::vline(s, 229, 50, EX - COIL / 2 - 50, 2, c::Ink);
  glt::vline(s, 241, 62, EX - COIL / 2 - 62, 2, c::Ink);
  glt::hline(s, 229, 50, 52, 2, c::Ink);
  glt::arrow(s, 290, 50, Dir::Right, c::Ink);
  glt::hline(s, 241, 62, 37, 2, c::Ink);
  glt::arrow(s, 276, 62, Dir::Left, c::Ink);
  glt::tag(s, "zum Verdichter !", 296, 42);
*/
}

void RltScreen::buildExhaust() {
  auto* s = screen_;
  // Fortluftklappe M06
  {
    Item& it = symbol(Pt::M06, Kind::Damper, 60, EX, COIL);
    it.sym.addTag("M06", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_BOTTOM_MID, 0, 4, 50);
  }
  // Abluftventilator M02 mit S02, S06, FU U02
  bracket(120, 170, 66, EX);
  {
    Item& it = symbol(Pt::M02, Kind::Fan, 145, EX, 36, Orientation::Left);
    glt::tag(s, "M02", 106, 142);
    it.sym.addValue(LV_ALIGN_OUT_BOTTOM_LEFT, 15, 3, 46);
  }
  
  symbol(Pt::S02, Kind::Differential, 145, 66, 22).sym.addTag("S02", LV_ALIGN_OUT_TOP_MID, 0, -2);
  //{
  //  Item& it = symbol(Pt::S06, Kind::Box, 205, 150, 18);
  //  it.sym.setMark("S");
  //  glt::tag(s, "S06", 218, 142);
  //}
  {
    Item& it = symbol(Pt::U02, Kind::Box, 157, 182, 28);
    it.sym.setMark("FU");
    glt::tag(s, "U02 / Y06", 176, 175);
  }
  // Zusatzregister und WRG-Abluftregister
  //decor(s, Kind::Cooling, 235, EX, "-");
  decor(s, Kind::Cooling, 320, EX, "-").addTag("WRG", LV_ALIGN_OUT_TOP_MID, 0, -2);
  // B06 am WRG-Bereich
  sensorBelow(Pt::B06, Kind::Temperature, 355, EX, SENSOR);
  {
    Item& it = items_[count_ - 1];
    it.sym.addTag("B06", LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    it.sym.addValue(LV_ALIGN_OUT_RIGHT_MID, 5, 0, 58);
  }
  // Filter F5 mit S04
  bracket(420, 470, 66, EX);
  decor(s, Kind::Filter, 445, EX, "F5");
  {
    Item& it = symbol(Pt::S04, Kind::Differential, 445, 66, 22);
    it.sym.addTag("S04", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_RIGHT_MID, 30, 0, 50);
  }
  // Druck B05 (Ist + Soll) und Druckkontakt S08
  sensorAbove(Pt::B05, Kind::Pressure, 600, EX, SENSOR);
  {
    Item& it = items_[count_ - 1];
    it.sym.addTag("B05", LV_ALIGN_OUT_LEFT_MID, -4, 0);
    it.sym.addValue(LV_ALIGN_OUT_TOP_MID, 0, -11, 58);
    it.setpoint = glt::valueBox(s, 600 - 29, 40, 58, true);
  }
  sensorAbove(Pt::S08, Kind::Box, 650, EX, 18);
  items_[count_ - 1].sym.setMark("P");
  items_[count_ - 1].sym.addTag("S08", LV_ALIGN_OUT_TOP_MID, 0, -2);
  // WRG-Pumpe und -Ventil (auf dem blauen Strang)
  {
    Item& it = symbol(Pt::M04, Kind::Pump, 310, 175, 22, Orientation::Up);
    it.sym.addTag("M04", LV_ALIGN_OUT_LEFT_MID, -4, 0);
  }
  {
    symbol(Pt::Y04, Kind::Valve, 310, 222, 24);
    glt::tag(s, "Y04", 268, 204);
    Item& it = items_[count_ - 1];
    it.sym.addValue(LV_ALIGN_OUT_LEFT_TOP, -6, 18, 56);
  }
}

void RltScreen::buildSupply() {
  auto* s = screen_;
  // Aussenluftklappe M05
  {
    Item& it = symbol(Pt::M05, Kind::Damper, 60, SU, COIL);
    it.sym.addTag("M05", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_BOTTOM_MID, 0, 4, 50);
  }
  // Filter F7 mit S03
  bracket(180, 230, 244, SU);
  decor(s, Kind::Filter, 205, SU, "F7");
  {
    Item& it = symbol(Pt::S03, Kind::Differential, 205, 244, 22);
    it.sym.addTag("S03", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_TOP_MID, 0, -20, 50);
  }
  // WRG-Zuluftregister
  decor(s, Kind::Heating, 320, SU, "+");
  // Heizregister mit M03 / Y01
  decor(s, Kind::Heating, 375, SU, "+");
  symbol(Pt::M03, Kind::Pump, 367, 345, 22, Orientation::Up).sym.addTag("M03", LV_ALIGN_OUT_LEFT_MID, -4, 0);
  {
    Item& it = symbol(Pt::Y01, Kind::Valve, 383, 385, 24);
    glt::tag(s, "Y01", 326, 368);
    it.sym.addValue(LV_ALIGN_OUT_LEFT_BOTTOM, -20, 20, 56);
  }
  // Temperatur nach Heizregister B04 (Ist + Soll), Frostschutz F01
  sensorBelow(Pt::B04, Kind::Temperature, 405, SU, SENSOR);
  {
    Item& it = items_[count_ - 1];
    glt::tag(s, "B04", 392, 324);
    it.sym.addValue(LV_ALIGN_OUT_RIGHT_TOP, 5, 1, 58);
    it.setpoint = glt::valueBox(s, 420, 322, 58, true);
  }
  sensorAbove(Pt::F01, Kind::Box, 420, SU, 18);
  items_[count_ - 1].sym.setMark("F");
  items_[count_ - 1].sym.addTag("F01", LV_ALIGN_OUT_TOP_MID, 0, -2);
  // Kuehlregister mit S09, Y02, E01
  bracket(478, 522, 244, SU);
  decor(s, Kind::Cooling, 500, SU, "-");
  {
    Item& it = symbol(Pt::S09, Kind::Differential, 500, 244, 22);
    it.sym.addTag("S09", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_TOP_MID, 0, -20, 50);
  }
  {
    Item& it = symbol(Pt::Y02, Kind::Valve, 492, 385, 24);
    glt::tag(s, "Y02", 452, 368);
    it.sym.addValue(LV_ALIGN_OUT_LEFT_BOTTOM, -2, 20, 52);
  }
  symbol(Pt::E01, Kind::Compressor, 508, 345, 22).sym.addTag("E01", LV_ALIGN_OUT_RIGHT_MID, 4, 0);
  // Zuluftventilator M01 mit S01, S05, FU U01
  bracket(547, 583, 244, SU);
  {
    Item& it = symbol(Pt::M01, Kind::Fan, 565, SU, 36, Orientation::Right);
    glt::tag(s, "M01", 526, 312);
    it.sym.addValue(LV_ALIGN_OUT_BOTTOM_LEFT, 13, 4, 46);
  }
  symbol(Pt::S01, Kind::Differential, 565, 244, 22).sym.addTag("S01", LV_ALIGN_OUT_TOP_MID, 0, -2);
  {
    Item& it = symbol(Pt::U01, Kind::Box, 580, 357, 28);
    it.sym.setMark("FU");
    glt::tag(s, "U01 / Y05", 598, 350);
  }
  {
    Item& it = symbol(Pt::S05, Kind::Box, 622, 321, 18);
    it.sym.setMark("S");
    glt::tag(s, "S05", 636, 313);
  }
  // Druck B02 (Ist + Soll) und Druckkontakt S07
  sensorAbove(Pt::B02, Kind::Pressure, 615, SU, SENSOR);
  {
    Item& it = items_[count_ - 1];
    it.sym.addTag("B02", LV_ALIGN_OUT_TOP_MID, 0, -2);
    it.sym.addValue(LV_ALIGN_OUT_TOP_MID, 0, -20, 58);
    it.setpoint = glt::valueBox(s, 615 - 29, 201, 58, true);
  }
  sensorAbove(Pt::S07, Kind::Box, 665, SU, 18);
  items_[count_ - 1].sym.setMark("P");
  items_[count_ - 1].sym.addTag("S07", LV_ALIGN_OUT_TOP_MID, 0, -2);
  // Nachheizregister mit Y03
  /*
  decor(s, Kind::Heating, 700, SU, "+");
  {
    Item& it = symbol(Pt::Y03, Kind::Valve, 708, 390, 24);
    glt::tag(s, "Y03", 722, 378);
    it.sym.addValue(LV_ALIGN_OUT_RIGHT_BOTTOM, 6, 26, 58);
  }
  */
  // Zulufttemperatur B03 (Ist + Soll)
  sensorBelow(Pt::B03, Kind::Temperature, 750, SU, SENSOR);
  {
    Item& it = items_[count_ - 1];
    it.sym.addTag("B03", LV_ALIGN_OUT_RIGHT_MID, 10, 2);
    it.sym.addValue(LV_ALIGN_OUT_BOTTOM_MID, 0, 4, 58);
    it.setpoint = glt::valueBox(s, 750 - 29, 346, 58, true);
  }
}

void RltScreen::update(const model::Snapshot& s) {
  char buf[24];
  for (int i = 0; i < count_; i++) {
    Item& it = items_[i];
    const auto& d = s[it.pt];
    const auto& inf = model::info(it.pt);
    if (it.hasSymbol) it.sym.setState(toState(it.pt, d));
    if (inf.hasValue) {
      model::formatValue(buf, sizeof buf, it.pt, model::usable(d) ? d.actual : model::Value{});
      it.sym.setValue(buf);
    }
    if (it.setpoint) {
      model::formatValue(buf, sizeof buf, it.pt, model::usable(d) ? d.setpoint : model::Value{});
      lv_label_set_text(glt::valueBoxLabel(it.setpoint), buf);
    }
  }

  // Kopfzeile: Hauptschaltbefehl und BMZ (Text + Farbe)
  const auto& hsb = s[Pt::HSB];
  bool hsbOk = hsb.quality == model::Quality::Good;
  lv_label_set_text_fmt(hsbLabel_, "HSB  %s | %s", hsbOk ? model::modeText(s.mode) : "?",
                        hsbOk ? model::statusText(Pt::HSB, hsb.status) : "?");
  lv_obj_set_style_bg_color(hsbBox_, lv_color_hex(!hsbOk ? c::StateUnknown
                                                  : hsb.status == model::Status::On ? c::StateRunning
                                                                                    : c::StateOff), 0);
  const auto& bmz = s[Pt::BMZ];
  bool bmzOk = bmz.quality == model::Quality::Good;
  bool fire = bmz.status == model::Status::Fault || bmz.status == model::Status::On;
  lv_label_set_text_fmt(bmzLabel_, "BMZ  %s", !bmzOk ? "? unbekannt" : fire ? "! AUSGELOEST" : "keine Ausloesung");
  lv_obj_set_style_bg_color(bmzBox_, lv_color_hex(!bmzOk ? c::StateUnknown : fire ? c::StateFault : c::Panel), 0);
}

}  // namespace ui
