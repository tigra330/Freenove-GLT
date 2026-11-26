#include "demo_source.h"

#include <math.h>

namespace model {
namespace {

constexpr uint32_t kPeriodMs = 1000;
const Pt kCommErrorPoints[] = {Pt::B03, Pt::B05, Pt::Y03, Pt::M04};

void set(Snapshot& s, Pt p, Status st, float v = NAN, float sp = NAN) {
  DataPoint& d = s[p];
  d.status = st;
  d.quality = Quality::Good;
  d.actual.valid = !isnan(v);
  d.actual.v = d.actual.valid ? v : 0.0f;
  d.setpoint.valid = !isnan(sp);
  d.setpoint.v = d.setpoint.valid ? sp : 0.0f;
}

// Leichte, rein kosmetische Schwankung, damit Aktualisierungen sichtbar sind.
float wobble(uint32_t t, float amp, float phase) {
  return amp * sinf((float)t / 4000.0f + phase);
}

}  // namespace

void DemoSource::nextScenario() {
  scenario_ = static_cast<Scenario>((static_cast<uint8_t>(scenario_) + 1) %
                                    static_cast<uint8_t>(Scenario::Count));
  dirty_ = true;
}

const char* DemoSource::scenarioText() const {
  switch (scenario_) {
    case Scenario::Normal: return "Normalbetrieb";
    case Scenario::FanFault: return "Stoerung M01";
    case Scenario::CommError: return "Komm.-fehler";
    case Scenario::PlantOff: return "Anlage AUS";
    default: return "?";
  }
}

bool DemoSource::poll(uint32_t now, Snapshot& s) {
  if (!dirty_ && now - lastMs_ < kPeriodMs) return false;
  lastMs_ = now;
  dirty_ = false;

  const bool off = scenario_ == Scenario::PlantOff;
  const Status run = off ? Status::Off : Status::On;
  const float w = off ? 0.0f : 1.0f;

  s = Snapshot{};
  s.sequence = ++sequence_;
  s.timestampMs = now;
  s.demo = true;
  s.mode = OperatingMode::Auto;

  set(s, Pt::HSB, run);
  set(s, Pt::BMZ, Status::Off);
  set(s, Pt::M01, run, 65.0f * w);
  set(s, Pt::M02, run, 62.0f * w);
  set(s, Pt::M03, run);
  set(s, Pt::M04, run);
  set(s, Pt::M05, run, 100.0f * w);
  set(s, Pt::M06, run, 100.0f * w);
  set(s, Pt::Y01, run, off ? 0.0f : 42.5f + wobble(now, 1.5f, 0.0f));
  set(s, Pt::Y02, Status::Off, 0.0f);
  set(s, Pt::Y03, run, off ? 0.0f : 12.0f);
  set(s, Pt::Y04, run, off ? 0.0f : 78.0f);
  set(s, Pt::E01, Status::Off);
  set(s, Pt::U01, run);
  set(s, Pt::U02, run);
  set(s, Pt::B02, Status::On, off ? 0.0f : 298.0f + wobble(now, 3.0f, 1.0f), 300.0f);
  set(s, Pt::B03, Status::On, 21.8f + wobble(now, 0.2f, 2.0f), 22.0f);
  set(s, Pt::B04, Status::On, 23.7f + wobble(now, 0.2f, 3.0f), 20.0f);
  set(s, Pt::B05, Status::On, off ? 0.0f : 292.0f + wobble(now, 3.0f, 4.0f), 300.0f);
  set(s, Pt::B06, Status::On, 14.4f);
  set(s, Pt::S01, run);
  set(s, Pt::S02, run);
  set(s, Pt::S03, Status::On, off ? 0.0f : 118.0f);
  set(s, Pt::S04, Status::On, off ? 0.0f : 95.0f);
  set(s, Pt::S05, Status::Off);
  set(s, Pt::S06, Status::Off);
  set(s, Pt::S07, Status::Off);
  set(s, Pt::S08, Status::Off);
  set(s, Pt::S09, Status::On, off ? 0.0f : 46.0f);
  set(s, Pt::F01, Status::Off);

  switch (scenario_) {
    case Scenario::FanFault:
      set(s, Pt::M01, Status::Fault, 0.0f);
      set(s, Pt::U01, Status::Fault);
      set(s, Pt::S01, Status::Off);
      break;
    case Scenario::CommError:
      for (Pt p : kCommErrorPoints) {
        DataPoint& d = s[p];
        d.quality = Quality::CommError;
        d.status = Status::Unknown;
        d.actual.valid = false;  // kein Wert -> Anzeige "---", niemals 0
      }
      s[Pt::B02].quality = Quality::Uncertain;
      break;
    default:
      break;
  }
  return true;
}

}  // namespace model
