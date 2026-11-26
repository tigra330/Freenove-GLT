#pragma once
// Zentrale Farbpalette der GLT-Bibliothek.
// Die Medienfarben orientieren sich an der Siemens-Referenzansicht des Projekts;
// sie sind eine Projektkonvention und keine verbindliche Norm.
#include <stdint.h>

namespace glt {
namespace color {

// Allgemein
constexpr uint32_t Ink = 0x344454;         // Konturen, Text
constexpr uint32_t InkMuted = 0x6b7885;    // Nebentexte
constexpr uint32_t Background = 0xe9edf1;  // Bildschirmhintergrund
constexpr uint32_t Panel = 0xffffff;       // Kacheln, Wertefelder
constexpr uint32_t PanelBorder = 0xc2cbd3;
constexpr uint32_t Accent = 0x2f6fb0;      // aktive Navigation

// Betriebszustand von Symbolen
constexpr uint32_t StateRunning = 0x42c963;
constexpr uint32_t StateOff = 0xbabfc3;
constexpr uint32_t StateFault = 0xe74e48;
constexpr uint32_t StateUnknown = 0xf0aa40;
constexpr uint32_t StateNormal = 0xffffff;  // Messstelle ohne Befund

// Luftkanaele
constexpr uint32_t AirOutdoor = 0x5fd35a;   // Aussenluft
constexpr uint32_t AirExhaust = 0xd4e617;   // Abluft / Fortluft
constexpr uint32_t AirPreheated = 0xe5372f; // kurzer Abschnitt nach WRG
constexpr uint32_t AirSupply = 0xdc46dc;    // Zuluft

// Rohrleitungen
constexpr uint32_t PipeHeatFlow = 0xe0342e;
constexpr uint32_t PipeHeatReturn = 0x2f4fd6;
constexpr uint32_t PipeColdFlow = 0x3cc6e6;
constexpr uint32_t PipeColdReturn = 0x2f4fd6;

// Werteanzeigen
constexpr uint32_t Setpoint = 0x7fdcf2;     // Sollwertfeld
constexpr uint32_t Demo = 0xf08c00;         // DEMO-Kennzeichnung

}  // namespace color
}  // namespace glt
