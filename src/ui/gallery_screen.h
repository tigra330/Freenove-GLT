#pragma once
#include <lvgl.h>

namespace ui {

// Symbolgalerie aus Version 0.1 (Kacheldarstellung, statische Demo-Werte).
class GalleryScreen {
 public:
  void create();
  lv_obj_t* screen() const { return screen_; }

 private:
  lv_obj_t* screen_ = nullptr;
};

}  // namespace ui
