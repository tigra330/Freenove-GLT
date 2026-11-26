#pragma once
// Synthetische Demo-Datenquelle. Keine Regelung, keine Simulation: feste
// Beispielwerte mit leichter Schwankung, plus manuell umschaltbare Szenarien
// zum Pruefen der Darstellung (Stoerung, Kommunikationsfehler, Anlage AUS).
#include "rlt_data.h"

namespace model {

class DemoSource : public DataSource {
 public:
  enum class Scenario : uint8_t { Normal, FanFault, CommError, PlantOff, Count };

  bool poll(uint32_t nowMs, Snapshot& out) override;
  const char* name() const override { return "Demo"; }

  void nextScenario();
  Scenario scenario() const { return scenario_; }
  const char* scenarioText() const;

 private:
  Scenario scenario_ = Scenario::Normal;
  uint32_t lastMs_ = 0;
  uint32_t sequence_ = 0;
  bool dirty_ = true;
};

}  // namespace model
