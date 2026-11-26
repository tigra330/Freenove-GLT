#include "rlt_data.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

namespace model {
namespace {

#define DEG "\xC2\xB0"  // Gradzeichen (UTF-8), in Montserrat enthalten

using K = PointKind;
// Signaltypen der S-Kontakte sind noch nicht festgelegt (spaeteres API-Mapping).
const PointInfo kInfo[kPointCount] = {
    {"HSB", "Hauptschaltbefehl", K::Command, "", 0, false, false},
    {"BMZ", "BMZ-Kontakt", K::Contact, "", 0, false, false},
    {"M01", "Zuluftventilator", K::Fan, "%", 0, true, false},
    {"M02", "Abluftventilator", K::Fan, "%", 0, true, false},
    {"M03", "Pumpe Heizregister", K::Pump, "", 0, false, false},
    {"M04", "Pumpe WRG", K::Pump, "", 0, false, false},
    {"M05", "Aussenluftklappe", K::Damper, "%", 0, true, false},
    {"M06", "Fortluftklappe", K::Damper, "%", 0, true, false},
    {"Y01", "Stellventil Heizregister", K::Valve, "%", 1, true, false},
    {"Y02", "Stellventil Kuehlregister", K::Valve, "%", 1, true, false},
    {"Y03", "Stellventil Nachheizregister", K::Valve, "%", 1, true, false},
    {"Y04", "Stellventil WRG", K::Valve, "%", 1, true, false},
    {"E01", "Verdichter", K::Compressor, "", 0, false, false},
    {"U01", "FU Zuluftventilator (Y05)", K::Converter, "", 0, false, false},
    {"U02", "FU Abluftventilator (Y06)", K::Converter, "", 0, false, false},
    {"B02", "Kanaldruck Zuluft", K::Pressure, "Pa", 0, true, true},
    {"B03", "Zulufttemperatur", K::Temperature, DEG "C", 1, true, true},
    {"B04", "Temperatur nach Heizregister", K::Temperature, DEG "C", 1, true, true},
    {"B05", "Kanaldruck Abluft", K::Pressure, "Pa", 0, true, true},
    {"B06", "Temperatur WRG-Bereich", K::Temperature, DEG "C", 1, true, false},
    {"S01", "Differenzdruck Zuluftventilator", K::DiffPressure, "", 0, false, false},
    {"S02", "Differenzdruck Abluftventilator", K::DiffPressure, "", 0, false, false},
    {"S03", "Differenzdruck Filter F7", K::DiffPressure, "Pa", 0, true, false},
    {"S04", "Differenzdruck Filter F5", K::DiffPressure, "Pa", 0, true, false},
    {"S05", "Kontakt S05 Zuluftventilator", K::Contact, "", 0, false, false},
    {"S06", "Kontakt S06 Abluftventilator", K::Contact, "", 0, false, false},
    {"S07", "Druckkontakt S07 Zuluft", K::Contact, "", 0, false, false},
    {"S08", "Druckkontakt S08 Abluft", K::Contact, "", 0, false, false},
    {"S09", "Differenzdruck Kuehlregister", K::DiffPressure, "Pa", 0, true, false},
    {"F01", "Frostschutz", K::Contact, "", 0, false, false},
};

}  // namespace

const PointInfo& info(Pt p) { return kInfo[static_cast<size_t>(p)]; }

void formatValue(char* buf, size_t n, Pt p, const Value& v) {
  if (!n) return;
  const PointInfo& i = info(p);
  if (!v.valid || isnan(v.v)) {
    snprintf(buf, n, "---");
    return;
  }
  char num[16];
  snprintf(num, sizeof num, "%.*f", i.decimals, (double)v.v);
  for (char* c = num; *c; ++c)
    if (*c == '.') *c = ',';
  if (i.unit[0])
    snprintf(buf, n, "%s %s", num, i.unit);
  else
    snprintf(buf, n, "%s", num);
}

const char* statusText(Pt p, Status s) {
  const PointKind k = info(p).kind;
  switch (s) {
    case Status::Unknown: return "unbekannt";
    case Status::Fault: return k == PointKind::Contact ? "ausgeloest" : "Stoerung";
    case Status::On:
      if (k == PointKind::Damper) return "offen";
      if (k == PointKind::Contact) return "aktiv";
      return "EIN";
    case Status::Off:
      if (k == PointKind::Damper) return "zu";
      if (k == PointKind::Contact) return p == Pt::BMZ || p == Pt::F01 ? "keine Ausloesung" : "inaktiv";
      return "AUS";
  }
  return "?";
}

const char* qualityText(Quality q) {
  switch (q) {
    case Quality::Good: return "gut";
    case Quality::Uncertain: return "unsicher";
    case Quality::CommError: return "Kommunikationsfehler";
    default: return "unbekannt";
  }
}

const char* kindText(PointKind k) {
  switch (k) {
    case PointKind::Fan: return "Ventilator";
    case PointKind::Pump: return "Pumpe";
    case PointKind::Damper: return "Luftklappe";
    case PointKind::Valve: return "Stellventil";
    case PointKind::Compressor: return "Verdichter";
    case PointKind::Converter: return "Frequenzumrichter";
    case PointKind::Temperature: return "Temperaturfuehler";
    case PointKind::Pressure: return "Druckfuehler";
    case PointKind::DiffPressure: return "Differenzdruck";
    case PointKind::Contact: return "Meldekontakt";
    case PointKind::Command: return "Schaltbefehl";
  }
  return "?";
}

const char* modeText(OperatingMode m) {
  switch (m) {
    case OperatingMode::Auto: return "AUTO";
    case OperatingMode::Manual: return "HAND";
    default: return "---";
  }
}

}  // namespace model
