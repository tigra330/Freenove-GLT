# AGENTS.md – Freenove-GLT

## Auftrag für Claude in VS Code

Entwickle das vorhandene PlatformIO-Projekt Freenove-GLT weiter. Die Symbol-Demo Version 0.1 läuft bereits erfolgreich auf dem physischen Display. Springe direkt zur vollständigen RLT-Anlagenvisualisierung; keine separate Entwicklungsstufe für Luftkanäle. Zeichne die benötigten Kanäle und Rohrleitungen als Teil des Anlagenschemas.

Das Display ist ausschließlich eine GLT-Bedien- und Visualisierungseinheit (HMI). Die eigentliche DDC, PID-Regelung, Verriegelung und Anlagensteuerung werden später auf einem separaten Controller entwickelt. Keine Regelungslogik, keine automatischen Anlagenreaktionen und keine Hardwareausgänge für Anlagenkomponenten implementieren. Demo-Werte sind reine Anzeigedaten.

## Bestand und technische Vorgaben

- Freenove FNK0115Q, 5 Zoll IPS, 800 × 480, RGB565, GT911.
- ESP32-S3 N16R8, 16 MB Flash, 8 MB Octal-PSRAM, Arduino via pioarduino.
- LVGL 9.5.0, Arduino GFX 1.6.5, TAMC_GT911 1.0.2.
- `platformio.ini`, `include/lv_conf.h` und `lib/board_fnk0115q/` sind funktionierend. Nicht ohne nachgewiesenen Grund ändern; Pins, Timings, Buffer, Touch, PSRAM und USB-CDC-Konfiguration beibehalten.
- `src/main.cpp`: ruft `lv_init()`, `board::begin()`, `showGallery()`, `lv_timer_handler()` und `board::setBrightness(30)` auf. Loop-Task-Stack 24 KB, LVGL-Handler im Loop mit 5 ms Delay.
- `lib/lvgl-glt/src/glt.h` und `glt.cpp`: vorhandene Klasse `glt::Symbol`, `create(parent, kind, x, y, size, name)`, `setState()`, `setValue()`, `object()`. `Kind`: Fan, Pump, Heating, Cooling, Filter, Damper, Valve, Temperature, Pressure, Differential. `State`: Off, Running, Fault, Unknown.
- Vorhandene Symbol-Implementierung ist für Kacheln ausgelegt (Root-Größe size+72 × size+52, Symbolkörper versetzt). Für kompakte Anlagenschemata eine rückwärtskompatible Variante mit frei positionierbaren Beschriftungen, kleinerem Footprint und ggf. Orientierung ergänzen. Bestehende Galerie weiterhin lauffähig halten.
- Native LVGL-9-API verwenden; keine LVGL-8-Aufrufe, keine externen Grafikbibliotheken oder unnötigen Abhängigkeiten. Keine temporären Punktarrays an LVGL übergeben, wenn LVGL deren Speicher weiter referenziert.

## Referenz und Anlagenaufbau

Referenz: Siemens-GLT-Screenshot `Bildschirmfoto vom 2026-11-24 14-28-23.png` (vom Benutzer bereitgestellt; falls im Workspace nicht vorhanden, dient folgende Beschreibung als Grundlage). Das Layout orientiert sich daran, nicht an einer frei erfundenen Anlage. Unklare Details nicht als gesicherte Anlagenfunktion darstellen.

Zwei horizontale Luftkanäle, rechts verbunden durch einen symbolischen Raum „Innenzone BT C“:

- Oben Abluft/Fortluft, Strömung rechts nach links: Raum AB → Druckmessung B05/S08 → Filter F5 mit S04 → WRG-Abluftregister → zusätzliches Abluftregister (im Bild mit Verbindung „zum Verdichter !“) → Abluftventilator M02 mit S02, S06 und FU U02/Y06 → Fortluftklappe M06 → FO.
- Unten Außenluft/Zuluft, Strömung links nach rechts: AU → Außenluftklappe M05 → Filter F7 mit S03 → WRG-Zuluftregister → Heizregister → Temperatur B04/Frostschutz F01 → Kühlregister mit S09 → Zuluftventilator M01 mit S01, S05 und FU U01/Y05 → Druck B02/S07 → Nachheizregister → Temperatur B03 → Raum ZL.
- WRG als zwei Register mit rotem/blauem Rohrkreis, Pumpe M04 und Stellventil Y04 darstellen. B06 am oberen WRG-Bereich.
- Heizregister mit Pumpe M03 und Ventil Y01; Kühlregister mit Y02 und Verdichter E01; Nachheizregister mit Y03. Rohranschlüsse nach unten mit Pfeilen. Die genaue hydraulische Funktion ist nicht vollständig aus dem Bild ableitbar; keine Regelung daraus ableiten.
- Hauptschaltbefehl AUTO/EIN sowie BMZ-Kontakt im Kopfbereich als Status anzeigen.
- Luftkanalfarben analog Vorlage: Außenluft grün, Abluft/Fortluft gelb, kurzer Abschnitt nach WRG rot, Zuluft magenta; Heizungsleitungen rot/blau, Kälte cyan/blau. Farben zentral definieren; keine Behauptung einer verbindlichen Norm.
- Aktive Komponenten grün, aus grau, Störung rot, unbekannt/Kommunikationsfehler deutlich gekennzeichnet (zusätzlich Text oder Symbol, nicht nur Farbe).

## Layout und Bedienung

- Klassischer flacher 2D-GLT-Stil, heller Hintergrund, dunkle Konturen, keine Symbolkacheln innerhalb des Anlagenschemas.
- RLT-Ansicht als Startseite. Kopfzeile ca. 36 px mit Anlagenname und DEMO-Kennzeichnung; Fußnavigation ca. 36–44 px. Rest für Schema nutzen. Empfohlene Kanalachsen etwa y=125 und y=285, Raum rechts; genaue Positionen an Lesbarkeit anpassen.
- Luftkanäle und Rohre zuerst zeichnen (Hintergrund), danach Komponenten, Sensoren und Werte. Keine horizontale/vertikale Scrollfläche in der Anlagenübersicht. Luftstromrichtung eindeutig kennzeichnen.
- Kleine kompakte Symbole, Betriebsmittelkennzeichen und Messwerte ohne Überlappung. Schriftgrößen an 800 × 480 anpassen, vorhandene aktivierte Fonts prüfen. Bei Platzmangel Details in Touch-Popup verschieben.
- Navigation zwischen RLT-Anlage und bisheriger Symbolgalerie. Keine leeren Dummy-Menüs für noch nicht implementierte Funktionen nötig.
- Antippen von Ventilator, Ventil, Klappe oder Sensor öffnet ein einfaches Detailfenster mit Kennzeichen, Typ, Istwert, ggf. Sollwert, Betriebsstatus und Datenqualität. Touchflächen ausreichend groß bzw. sinnvoll gruppiert. Demo-Änderungen optional lokal und deutlich als Demo markieren; keine realen Schreibbefehle in dieser Version.

## Datenmodell und Demo

Visualisierung und Datenquelle strikt entkoppeln. Eine kleine typisierte Datenpunktverwaltung oder Anlagen-Snapshot-Struktur erstellen, ohne HTTP-Client in diesem Schritt. Einheiten, IDs, Istwerte, optionale Sollwerte, Status und Qualität getrennt von LVGL halten. Nicht vorhandene Werte als unbekannt behandeln, niemals als Null interpretieren. UI aus Snapshot aktualisieren, nicht aus hardcodierten Labels in der Zeichenroutine.

Beispielwerte für eine laufende Demo (synthetisch, nicht reale Messwerte):

| ID | Bedeutung | Demo |
| --- | --- | --- |
| M01 / M02 | Zu-/Abluftventilator | EIN, 65 / 62 % |
| M05 / M06 | Außen-/Fortluftklappe | offen, 100 % |
| M03 / M04 | Heizungs-/WRG-Pumpe | EIN |
| B02 / B05 | Kanaldrücke | 298 / 292 Pa, Soll jeweils 300 Pa |
| B03 | Zulufttemperatur | 21,8 °C, Soll 22 °C |
| B04 | Temperatur nach Heizregister | 23,7 °C, Soll 20 °C |
| B06 | Temperatur am WRG-Bereich | 14,4 °C |
| Y01 / Y02 / Y03 / Y04 | Stellventile | 42,5 / 0 / 12 / 78 % |
| S03 / S04 | Filter-Differenzdruck | Beispielwerte, nur als Demo |
| E01 | Verdichter | AUS |
| F01 / BMZ | Frostschutz / Brandmeldung | keine Auslösung (Demo) |

S01–S09 und FU-Kennzeichen nach Vorlage soweit sinnvoll anzeigen; genaue Signaltypen später per API-Mapping festlegen. Demo-Daten müssen keine physikalisch konsistente Simulation darstellen. Keine PID-Regler, keine Druck-/Temperaturberechnung. Optional gezielte manuelle Demo-Statuswechsel zum Prüfen der Darstellung.

## Vorgeschlagene Struktur (an Bestand anpassen)

```text
src/
  main.cpp
  ui/
    rlt_screen.h/.cpp
    gallery_screen.h/.cpp
    detail_dialog.h/.cpp
  model/
    rlt_data.h/.cpp
    demo_source.h/.cpp
lib/lvgl-glt/src/
  glt.h/.cpp
  glt_colors.h
  ggf. glt_lines.h/.cpp
```

Die GLT-Bibliothek bleibt generisch und kennt weder die konkrete RLT noch BAScloud, MQTT oder Netzwerk. Leitungshelfer dürfen intern im RLT-Schema entstehen oder bei Wiederverwendbarkeit in der Bibliothek liegen; kein separater Leitungs-Demobildschirm. Für spätere REST-API eine Datenquellen-Schnittstelle bzw. klare Update-Grenze vorsehen. Keine API-Endpunkte, Zugangsdaten oder BAScloud-Datenpunkt-IDs erfinden. Spätere Netzwerkabfragen nicht im LVGL-Thread blockierend ausführen; LVGL-Updates ausschließlich im UI-Kontext.

## Arbeitsweise und Abnahme

1. Aktuelle Workspace-Dateien lesen und lokale Änderungen respektieren. Funktionierende Hardwareansteuerung nicht ersetzen.
2. RLT-Layout und kompakte Symbolvariante implementieren, vorhandene Galerie auslagern und erreichbar halten.
3. Demo-Snapshot, dynamische Werte und Statusfarben sowie Detaildialoge umsetzen. Objekte einmal erstellen, Werte aktualisieren statt Bildschirm periodisch neu aufzubauen. Screen-Wechsel, Callback-Lebensdauer und Symbol-Zeiger korrekt behandeln.
4. `pio run` (bzw. `pio run -e freenove_5inch`) ausführen und Fehler beheben, falls PlatformIO verfügbar. Nicht ungefragt flashen. Keine Behauptung eines Hardwaretests ohne tatsächlichen Test.
5. README aktualisieren: Aufbau, Bedienung, Datenmodell, Build und Upload, bekannte Einschränkungen. Geänderte Dateien und tatsächliches Build-Ergebnis zusammenfassen.

Akzeptanz: Projekt kompiliert; RLT ist Startansicht; zwei Luftstränge, WRG, Filter, Klappen, Ventilatoren, Heiz-/Kühlregister, Sensoren und Stellventile sind erkennbar; Beschriftungen und Werte überdecken keine wichtigen Symbole; DEMO ist sichtbar; Touch-Details und Galerie-Navigation funktionieren; Hardwareprofil bleibt unverändert; keine aktive Netzwerkkommunikation und keine Anlagenregelung. Sichtprüfung auf dem echten Display erfolgt durch den Benutzer.

## Nicht Teil dieses Auftrags

REST-API-Implementierung, MQTT/BACnet, echte Sollwert-Schreibbefehle, Alarme mit Quittierung, Trends, Anlageneditor, mehrere Anlagen, OTA-Änderungen und Entwicklung des externen RLT-Controllers. Diese Funktionen folgen erst nach erfolgreicher RLT-Visualisierung.
