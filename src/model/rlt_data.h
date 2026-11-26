#pragma once
// Datenpunktmodell der RLT-Anlage – unabhaengig von LVGL und Netzwerk.
// Die UI liest nur Snapshots; Datenquellen (Demo, spaeter REST) schreiben nur
// Snapshots. Fehlende Werte sind "ungueltig" und werden nie als 0 angezeigt.
#include <stddef.h>
#include <stdint.h>

namespace model {

enum class Quality : uint8_t { Unknown, Good, Uncertain, CommError };
enum class Status : uint8_t { Unknown, Off, On, Fault };
enum class OperatingMode : uint8_t { Unknown, Auto, Manual };

enum class PointKind : uint8_t {
  Fan, Pump, Damper, Valve, Compressor, Converter,
  Temperature, Pressure, DiffPressure, Contact, Command
};

// Feste Punktliste der Anlage. Reihenfolge = Index in Snapshot::points.
enum class Pt : uint8_t {
  HSB, BMZ,
  M01, M02, M03, M04, M05, M06,
  Y01, Y02, Y03, Y04,
  E01, U01, U02,
  B02, B03, B04, B05, B06,
  S01, S02, S03, S04, S05, S06, S07, S08, S09,
  F01,
  Count
};
constexpr size_t kPointCount = static_cast<size_t>(Pt::Count);

// Statische Beschreibung (Stammdaten) eines Datenpunkts.
struct PointInfo {
  const char* id;       // Betriebsmittelkennzeichen
  const char* name;     // Klartext
  PointKind kind;
  const char* unit;     // "" wenn ohne Analogwert
  uint8_t decimals;
  bool hasValue;        // Analogwert vorgesehen
  bool hasSetpoint;
};

struct Value {
  bool valid = false;
  float v = 0.0f;
};

// Laufzeitzustand eines Datenpunkts.
struct DataPoint {
  Status status = Status::Unknown;
  Quality quality = Quality::Unknown;
  Value actual;
  Value setpoint;
};

struct Snapshot {
  uint32_t sequence = 0;     // wird bei jeder Aenderung erhoeht
  uint32_t timestampMs = 0;  // Zeitpunkt der Erzeugung (Quelle)
  bool demo = false;         // synthetische Daten
  OperatingMode mode = OperatingMode::Unknown;  // Hauptschaltbefehl-Betriebsart
  DataPoint points[kPointCount];

  DataPoint& operator[](Pt p) { return points[static_cast<size_t>(p)]; }
  const DataPoint& operator[](Pt p) const { return points[static_cast<size_t>(p)]; }
};

const PointInfo& info(Pt p);

// Formatierung (deutsches Dezimalkomma, Einheit). Ungueltig -> "---".
void formatValue(char* buf, size_t n, Pt p, const Value& v);
const char* statusText(Pt p, Status s);
const char* qualityText(Quality q);
const char* kindText(PointKind k);
const char* modeText(OperatingMode m);

// Wert ist anzeigbar (gueltig und nicht als gestoert markiert).
inline bool usable(const DataPoint& d) {
  return d.quality == Quality::Good || d.quality == Quality::Uncertain;
}

// Schnittstelle fuer Datenquellen. poll() darf nicht blockieren und wird im
// UI-Kontext aufgerufen; eine spaetere Netzwerkquelle holt Daten in einem
// eigenen Task und kopiert hier nur den fertigen Snapshot.
class DataSource {
 public:
  virtual ~DataSource() = default;
  // true, wenn out aktualisiert wurde.
  virtual bool poll(uint32_t nowMs, Snapshot& out) = 0;
  virtual const char* name() const = 0;
};

}  // namespace model
