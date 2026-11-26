#include "app.h"

#include "detail_dialog.h"
#include "gallery_screen.h"
#include "rlt_screen.h"
#include "ui_common.h"

namespace ui {
namespace {

RltScreen g_rlt;
GalleryScreen g_gallery;
DetailDialog g_dialog;
model::DemoSource* g_source = nullptr;
model::Snapshot g_snapshot;  // letzter gueltiger Stand fuer UI und Dialog

void onPoint(model::Pt p) { g_dialog.open(p, g_snapshot); }

void onDemo() {
  if (!g_source) return;
  g_source->nextScenario();  // naechstes poll() liefert sofort neue Daten
  g_rlt.setDemoText(g_source->scenarioText());
}

}  // namespace

void navigate(ScreenId id) {
  g_dialog.close();
  lv_screen_load(id == ScreenId::Gallery ? g_gallery.screen() : g_rlt.screen());
}

namespace app {

void begin(model::DemoSource& source) {
  g_source = &source;
  g_rlt.create(onPoint, onDemo);
  g_gallery.create();
  g_dialog.create();
  g_rlt.setDemoText(source.scenarioText());
  g_rlt.update(g_snapshot);  // alles "unbekannt", bis Daten vorliegen
  lv_screen_load(g_rlt.screen());
}

void tick(uint32_t nowMs) {
  if (g_source && g_source->poll(nowMs, g_snapshot)) {
    g_rlt.update(g_snapshot);
    g_dialog.update(g_snapshot);
  }
}

}  // namespace app
}  // namespace ui
