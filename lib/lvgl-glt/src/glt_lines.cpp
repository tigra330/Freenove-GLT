#include "glt_lines.h"

#include "glt_colors.h"

namespace glt {
namespace {

lv_obj_t* plain(lv_obj_t* parent, int x, int y, int w, int h, uint32_t color) {
  auto* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, lv_color_hex(color), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_remove_flag(o, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
  return o;
}

// Statische Dreiecke (geschlossen), 12 px Kantenlaenge. Spitze jeweils bekannt.
constexpr int A = 12, H = 6;
const lv_point_precise_t kRight[] = {{0, 0}, {A, H}, {0, 2 * H}, {0, 0}};
const lv_point_precise_t kLeft[] = {{A, 0}, {0, H}, {A, 2 * H}, {A, 0}};
const lv_point_precise_t kDown[] = {{0, 0}, {2 * H, 0}, {H, A}, {0, 0}};
const lv_point_precise_t kUp[] = {{H, 0}, {2 * H, A}, {0, A}, {H, 0}};

}  // namespace

lv_obj_t* hline(lv_obj_t* parent, int x, int y, int len, int thick, uint32_t color) {
  return plain(parent, x, y - thick / 2, len, thick, color);
}

lv_obj_t* vline(lv_obj_t* parent, int x, int y, int len, int thick, uint32_t color) {
  return plain(parent, x - thick / 2, y, thick, len, color);
}

lv_obj_t* arrow(lv_obj_t* parent, int x, int y, Dir dir, uint32_t color) {
  auto* o = lv_line_create(parent);
  lv_obj_set_style_line_width(o, 2, 0);
  lv_obj_set_style_line_color(o, lv_color_hex(color), 0);
  lv_obj_set_style_line_rounded(o, true, 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  switch (dir) {
    case Dir::Right: lv_line_set_points(o, kRight, 4); lv_obj_set_pos(o, x - A, y - H); break;
    case Dir::Left: lv_line_set_points(o, kLeft, 4); lv_obj_set_pos(o, x, y - H); break;
    case Dir::Down: lv_line_set_points(o, kDown, 4); lv_obj_set_pos(o, x - H, y - A); break;
    case Dir::Up: lv_line_set_points(o, kUp, 4); lv_obj_set_pos(o, x - H, y); break;
  }
  return o;
}

lv_obj_t* tag(lv_obj_t* parent, const char* text, int x, int y) {
  auto* o = lv_label_create(parent);
  lv_label_set_text(o, text);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_style_text_color(o, lv_color_hex(color::Ink), 0);
  lv_obj_set_style_text_font(o, &lv_font_montserrat_14, 0);
  return o;
}

lv_obj_t* valueBox(lv_obj_t* parent, int x, int y, int w, bool setpoint) {
  auto* box = plain(parent, x, y, w, 18, setpoint ? color::Setpoint : color::Panel);
  lv_obj_set_style_border_width(box, 1, 0);
  lv_obj_set_style_border_color(box, lv_color_hex(color::Ink), 0);
  auto* l = lv_label_create(box);
  lv_label_set_text(l, "---");
  lv_obj_set_style_text_color(l, lv_color_hex(color::Ink), 0);
  lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
  lv_obj_center(l);
  return box;
}

lv_obj_t* valueBoxLabel(lv_obj_t* box) { return box ? lv_obj_get_child(box, 0) : nullptr; }

}  // namespace glt
