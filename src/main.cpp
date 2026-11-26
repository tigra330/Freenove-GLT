#include <Arduino.h>
#include <lvgl.h>

#include "board.h"
#include "model/demo_source.h"
#include "ui/app.h"

SET_LOOP_TASK_STACK_SIZE(24 * 1024);

static model::DemoSource demoSource;

void setup() {
  Serial.begin(115200);
  delay(200);
  lv_init();
  if (!board::begin()) {
    Serial.println("Board init failed");
    while (true) delay(1000);
  }
  ui::app::begin(demoSource);
  ui::app::tick(millis());
  lv_timer_handler();
  board::setBrightness(30);
  Serial.println("GLT RLT demo ready");
}

void loop() {
  ui::app::tick(millis());
  lv_timer_handler();
  delay(5);
}
