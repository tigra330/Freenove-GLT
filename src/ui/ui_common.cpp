#include "ui_common.h"

#include <stdint.h>

#include "glt.h"

namespace ui {
namespace {

void navCb(lv_event_t* e) {
  navigate(static_cast<ScreenId>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e))));
}

}  // namespace

lv_obj_t* makeScreen() {
  auto* s = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(s, lv_color_hex(glt::color::Background), 0);
  lv_obj_set_style_bg_opa(s, LV_OPA_COVER, 0);
  lv_obj_remove_flag(s, LV_OBJ_FLAG_SCROLLABLE);
  return s;
}

lv_obj_t* panel(lv_obj_t* parent, int x, int y, int w, int h, uint32_t bg, int border) {
  auto* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, lv_color_hex(bg), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(o, border, 0);
  lv_obj_set_style_border_color(o, lv_color_hex(glt::color::Ink), 0);
  lv_obj_remove_flag(o, (lv_obj_flag_t)(LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE));
  return o;
}

lv_obj_t* text(lv_obj_t* parent, const char* t, int x, int y, const lv_font_t* font, uint32_t color) {
  auto* l = lv_label_create(parent);
  lv_label_set_text(l, t);
  lv_obj_set_pos(l, x, y);
  lv_obj_set_style_text_font(l, font, 0);
  lv_obj_set_style_text_color(l, lv_color_hex(color), 0);
  return l;
}

lv_obj_t* button(lv_obj_t* parent, const char* t, int x, int y, int w, int h, lv_event_cb_t cb, void* user) {
  auto* b = lv_button_create(parent);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_radius(b, 4, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_set_style_bg_color(b, lv_color_hex(0x5d6d7c), 0);
  auto* l = lv_label_create(b);
  lv_label_set_text(l, t);
  lv_obj_set_style_text_font(l, &lv_font_montserrat_14, 0);
  lv_obj_center(l);
  if (cb) lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, user);
  return b;
}

lv_obj_t* footer(lv_obj_t* screen, ScreenId active) {
  auto* f = panel(screen, 0, kFooterY, 800, kFooterH, 0xd5dce3);
  lv_obj_set_style_border_width(f, 1, 0);
  lv_obj_set_style_border_side(f, LV_BORDER_SIDE_TOP, 0);
  lv_obj_set_style_border_color(f, lv_color_hex(glt::color::PanelBorder), 0);
  struct Nav { const char* t; ScreenId id; int x, w; };
  const Nav items[] = {{"RLT-Anlage", ScreenId::Rlt, 6, 120}, {"Symbolgalerie", ScreenId::Gallery, 132, 136}};
  for (const auto& n : items) {
    auto* b = button(f, n.t, n.x, 5, n.w, 34, navCb, reinterpret_cast<void*>(static_cast<intptr_t>(n.id)));
    if (n.id == active) lv_obj_set_style_bg_color(b, lv_color_hex(glt::color::Accent), 0);
  }
  return f;
}

}  // namespace ui
