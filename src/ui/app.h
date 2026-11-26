#pragma once
// UI-Anwendung: erzeugt alle Screens einmalig und verteilt Snapshots.
// Alle Funktionen nur im LVGL-Kontext (Arduino-loop) aufrufen.
#include <stdint.h>

#include "../model/demo_source.h"

namespace ui {
namespace app {

void begin(model::DemoSource& source);
// Fragt die Datenquelle ab (nicht blockierend) und aktualisiert die UI.
void tick(uint32_t nowMs);

}  // namespace app
}  // namespace ui
