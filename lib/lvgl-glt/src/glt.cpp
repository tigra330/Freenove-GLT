#include "glt.h"

namespace glt {
namespace {

constexpr uint32_t ink = color::Ink;

lv_obj_t* rect(lv_obj_t* p, int x, int y, int w, int h, uint32_t c, int border = 0, int radius = 0) {
  auto* o = lv_obj_create(p);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, lv_color_hex(c), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(o, border, 0);
  lv_obj_set_style_border_color(o, lv_color_hex(ink), 0);
  lv_obj_set_style_radius(o, radius, 0);
  lv_obj_remove_flag(o, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
  return o;
}

lv_obj_t* label(lv_obj_t* p, const char* text, int x, int y, int w) {
  auto* o = lv_label_create(p);
  lv_label_set_text(o, text);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_width(o, w);
  lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(o, lv_color_hex(ink), 0);
  lv_obj_set_style_text_font(o, &lv_font_montserrat_14, 0);
  return o;
}

// Zeichenstift im Symbolkoerper. Duenne gedrehte Rechtecke statt lv_line, damit
// keine temporaeren Punktarrays noetig sind. rot dreht die Zeichnung um die
// Koerpermitte (nur fuer runde, quadratische Koerper verwendet).
struct Pen {
  lv_obj_t* body;
  int w, h, rot;  // rot in Grad: 0, 90, 180, 270 (im Uhrzeigersinn)
  void bar(int x, int y, int len, int angle) const {
    int cx = w / 2, cy = h / 2, dx = x - cx, dy = y - cy;
    switch (rot) {
      case 90: x = cx - dy; y = cy + dx; break;
      case 180: x = cx - dx; y = cy - dy; break;
      case 270: x = cx + dy; y = cy - dx; break;
      default: break;
    }
    auto* o = rect(body, x, y, len, 2, ink);
    lv_obj_set_style_transform_pivot_x(o, 0, 0);
    lv_obj_set_style_transform_pivot_y(o, 1, 0);
    lv_obj_set_style_transform_rotation(o, angle + rot * 10, 0);
  }
};

int orientationDeg(Orientation o) {
  switch (o) {
    case Orientation::Down: return 90;
    case Orientation::Left: return 180;
    case Orientation::Up: return 270;
    default: return 0;
  }
}

}  // namespace

void Symbol::create(lv_obj_t* parent, Kind kind, int x, int y, int size, const char* name) {
  build(parent, kind, x, y, size, false, Orientation::Default);
  label(root_, name, 0, 0, size + 72);
  value_ = label(root_, "--", 0, size + 25, size + 72);
}

void Symbol::createCompact(lv_obj_t* parent, Kind kind, int cx, int cy, int size, Orientation o) {
  build(parent, kind, cx, cy, size, true, o);
  // Kleine Symbole bekommen eine groessere Touchflaeche.
  lv_obj_set_ext_click_area(root_, size < 30 ? 10 : 6);
}

void Symbol::build(lv_obj_t* parent, Kind kind, int x, int y, int s, bool compact, Orientation o) {
  kind_ = kind;
  compact_ = compact;
  bool round = kind == Kind::Fan || kind == Kind::Pump || kind == Kind::Temperature ||
               kind == Kind::Pressure || kind == Kind::Compressor;
  bool square = round || kind == Kind::Box || (compact && kind == Kind::Differential);
  int w = square ? s : s * 2 / 3;

  root_ = lv_obj_create(parent);
  lv_obj_remove_style_all(root_);
  lv_obj_remove_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(root_, LV_OBJ_FLAG_CLICKABLE);
  int a, b;
  if (compact) {
    lv_obj_set_pos(root_, x - w / 2, y - s / 2);
    lv_obj_set_size(root_, w, s);
    a = 0;
    b = 0;
  } else {
    lv_obj_set_pos(root_, x, y);
    lv_obj_set_size(root_, s + 72, s + 52);
    a = 36 + (s - w) / 2;
    b = 20;
  }
  body_ = rect(root_, a, b, w, s, color::Panel, 1, round ? LV_RADIUS_CIRCLE : 0);

  int base = kind == Kind::Pump ? 270 : 0;  // Pumpe zeigt ohne Drehung nach oben
  int rot = o == Orientation::Default ? 0 : (orientationDeg(o) - base + 360) % 360;
  Pen pen{body_, w, s, rot};

  switch (kind) {
    case Kind::Fan:
      pen.bar(s / 5, s / 4, s * 3 / 5, 230);
      pen.bar(s / 5, s * 3 / 4, s * 3 / 5, -230);
      pen.bar(s / 5, s / 4, s / 2, 900);
      break;
    case Kind::Pump:
      pen.bar(s / 5, s * 3 / 4, s * 3 / 5, -600);
      pen.bar(s / 2, s / 5, s * 3 / 5, 600);
      pen.bar(s / 5, s * 3 / 4, s * 3 / 5, 0);
      break;
    case Kind::Heating:
    case Kind::Cooling:
    case Kind::Filter:
      pen.bar(2, s - 3, s * 11 / 10, -560);
      if (kind == Kind::Filter) {
        pen.bar(2, 2, s * 11 / 10, 560);
        mark_ = label(body_, "F7", 0, s / 3, w);
        // Kennzeichen lesbar ueber den Diagonalen halten
        lv_obj_set_width(mark_, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(mark_, lv_color_hex(color::Panel), 0);
        lv_obj_set_style_bg_opa(mark_, LV_OPA_COVER, 0);
        lv_obj_set_style_pad_hor(mark_, 1, 0);
        lv_obj_align(mark_, LV_ALIGN_CENTER, 0, 0);
      } else {
        mark_ = label(body_, kind == Kind::Heating ? "+" : "-", 0, compact ? 1 : 3, w);
      }
      break;
    case Kind::Damper:
      for (int i = 1; i < 5; i++) pen.bar(2, i * s / 5, w - 4, 0);
      break;
    case Kind::Valve:
      pen.bar(2, s / 4, s * 4 / 5, 370);
      pen.bar(2, s * 3 / 4, s * 4 / 5, -370);
      pen.bar(2, s / 4, s / 2, 900);
      pen.bar(w - 3, s / 4, s / 2, 900);
      break;
    case Kind::Temperature: mark_ = label(body_, "T", 0, (s - 16) / 2, w); break;
    case Kind::Pressure: mark_ = label(body_, "P", 0, (s - 16) / 2, w); break;
    case Kind::Differential: mark_ = label(body_, "dP", 0, compact ? (s - 16) / 2 : s / 3, w); break;
    case Kind::Compressor:
      pen.bar(s / 2, s / 5, s * 3 / 5, 900);
      pen.bar(s / 5, s / 2, s * 3 / 5, 0);
      pen.bar(s / 4, s / 4, s * 7 / 10, 450);
      pen.bar(s / 4, s * 3 / 4, s * 7 / 10, -450);
      break;
    case Kind::Box: mark_ = label(body_, "", 0, (s - 16) / 2, w); break;
  }
  if (kind == Kind::Fan || kind == Kind::Pump || kind == Kind::Damper || kind == Kind::Valve ||
      kind == Kind::Compressor)
    setState(State::Off);
}

void Symbol::setState(State state) {
  if (!body_) return;
  uint32_t c = state == State::Running ? color::StateRunning
             : state == State::Fault   ? color::StateFault
             : state == State::Unknown ? color::StateUnknown
             : state == State::Normal  ? color::StateNormal
                                       : color::StateOff;
  lv_obj_set_style_bg_color(body_, lv_color_hex(c), 0);
  updateBadge(state);
}

void Symbol::updateBadge(State state) {
  bool show = state == State::Fault || state == State::Unknown;
  if (!show) {
    if (badge_) lv_obj_add_flag(badge_, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  if (!badge_) {
    // Geschwister des Symbols, damit es nicht vom Koerper abgeschnitten wird.
    badge_ = rect(lv_obj_get_parent(root_), 0, 0, 14, 14, color::StateFault, 1, 3);
    auto* t = lv_label_create(badge_);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(t, lv_color_hex(0xffffff), 0);
    lv_obj_center(t);
    lv_obj_align_to(badge_, body_, LV_ALIGN_OUT_RIGHT_TOP, -7, -7);
  }
  bool fault = state == State::Fault;
  lv_obj_set_style_bg_color(badge_, lv_color_hex(fault ? color::StateFault : color::StateUnknown), 0);
  auto* t = lv_obj_get_child(badge_, 0);
  lv_obj_set_style_text_color(t, lv_color_hex(fault ? 0xffffff : 0x1d2630), 0);
  lv_label_set_text(t, fault ? "!" : "?");
  lv_obj_remove_flag(badge_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(badge_);
}

void Symbol::setValue(const char* value) {
  if (!value_) return;
  // Kompakte Wertefelder sind Boxen mit Label-Kind.
  lv_obj_t* l = lv_obj_check_type(value_, &lv_label_class) ? value_ : lv_obj_get_child(value_, 0);
  if (l) lv_label_set_text(l, value);
}

void Symbol::setMark(const char* mark) {
  if (mark_) lv_label_set_text(mark_, mark);
}

lv_obj_t* Symbol::addTag(const char* text, lv_align_t align, int dx, int dy) {
  auto* t = tag(lv_obj_get_parent(root_), text, 0, 0);
  lv_obj_align_to(t, body_, align, dx, dy);
  return t;
}

lv_obj_t* Symbol::addValue(lv_align_t align, int dx, int dy, int width) {
  value_ = valueBox(lv_obj_get_parent(root_), 0, 0, width);
  lv_obj_align_to(value_, body_, align, dx, dy);
  return value_;
}

}  // namespace glt
