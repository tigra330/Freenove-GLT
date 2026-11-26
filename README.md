# Freenove-GLT 0.2 – RLT-Anlagenvisualisierung (DEMO)

GLT-Bedien- und Visualisierungseinheit (HMI) für das Freenove FNK0115Q (5" IPS, 800 × 480, GT911-Touch, ESP32-S3 N16R8) mit LVGL 9.5.0.

Das Display zeigt die Anlage nur an. Es gibt keine Regelung, keine Verriegelung, keine Schreibbefehle, keine Hardwareausgänge und keine Netzwerkkommunikation. Alle Werte sind synthetische Demo-Daten.

## Bedienung

- **Startansicht RLT-Anlage**: zwei Luftstränge (oben Abluft/Fortluft ←, unten Außenluft/Zuluft →), WRG mit Pumpe M04/Ventil Y04, Filter, Klappen, Ventilatoren mit FU, Heiz-, Kühl- und Nachheizregister mit Ventilen, Sensoren und Raum „Innenzone BT C“.
- **Kopfzeile**: Anlagenname, DEMO-Kennzeichnung, Hauptschaltbefehl (HSB) und BMZ-Kontakt. Beide Felder lassen sich antippen.
- **Detailfenster**: Ein Tipp auf einen Ventilator, eine Pumpe, ein Ventil, eine Klappe, einen Sensor, einen Kontakt oder einen FU öffnet ein Fenster mit Kennzeichen, Typ, Istwert, Sollwert, Status, Datenqualität und Quelle. Es schließt über „Schliessen“ oder einen Tipp neben das Fenster. Die Werte aktualisieren sich, solange das Fenster offen ist.
- **Fußleiste**: Wechsel zwischen „RLT-Anlage“ und „Symbolgalerie“. Der orange Button „Demo: …“ schaltet die Darstellungsszenarien weiter: Normalbetrieb → Störung M01 → Kommunikationsfehler → Anlage AUS.
- **Statusdarstellung**: grün = EIN/aktiv, grau = AUS, rot mit „!“ = Störung, orange mit „?“ = unbekannt, unsicher oder Kommunikationsfehler, weiß = Messstelle ohne Befund. Sollwertfelder sind cyan. Fehlt ein Wert, wird „---“ angezeigt, nie 0.

## Aufbau

```text
src/
  main.cpp               Board-Init, LVGL-Loop, Datenquelle -> UI
  model/rlt_data.*       Datenpunktliste (IDs, Einheiten, Typ), Snapshot, Formatierung,
                         DataSource-Schnittstelle (ohne LVGL)
  model/demo_source.*    Synthetische Demo-Werte + Szenarien
  ui/app.*               Erzeugt Screens einmalig, Navigation, Snapshot-Verteilung
  ui/rlt_screen.*        Anlagenschema (Kanäle/Rohre zuerst, dann Komponenten/Werte)
  ui/detail_dialog.*     Modales Detailfenster auf lv_layer_top()
  ui/gallery_screen.*    Symbolgalerie aus Version 0.1
  ui/ui_common.*         Screen, Buttons, Fußleiste
lib/lvgl-glt/src/        Generische GLT-Bibliothek (keine Anlagen-/Netzwerk-Kenntnis)
  glt.h/.cpp             glt::Symbol: Kachelvariante (unverändert) + createCompact()
                         mit Orientierung, setMark(), addTag()/addValue(), Statusbadge
  glt_colors.h           Zentrale Farben (Medien, Zustände)
  glt_lines.h/.cpp       Leitungen, Pfeilspitzen (statische Punktarrays), Tags, Wertefelder
lib/board_fnk0115q/      Hardwareprofil (unverändert)
tools/host_preview/      PC-Vorschau der UI als PNG (nicht Teil der Firmware)
```

## Datenmodell

- `model::Pt` ist die feste Punktliste: HSB, BMZ, M01–M06, Y01–Y04, E01, U01/U02, B02–B06, S01–S09 und F01. `model::info()` liefert die Stammdaten (Kennzeichen, Klartext, Typ, Einheit, Nachkommastellen, ob Analogwert und Sollwert vorgesehen sind).
- `model::Snapshot` enthält je Punkt den Status (Unknown/Off/On/Fault), die Qualität (Unknown/Good/Uncertain/CommError), den Istwert und den Sollwert (jeweils mit `valid`-Flag). Dazu kommen Betriebsart, Sequenz und Demo-Kennung.
- Die UI liest ausschließlich Snapshots (`RltScreen::update`). Sie erzeugt keine Objekte neu, sondern setzt nur Zustände und Texte.
- Spätere Datenquellen implementieren `model::DataSource::poll()`. Das muss nicht blockierend im UI-Kontext laufen: Ein REST-Client holt die Daten in einem eigenen Task und kopiert hier nur den fertigen Snapshot. Die Signaltypen der S-Kontakte und das API-Mapping sind noch offen.

## Build und Upload

In VS Code den Ordner als PlatformIO-Projekt öffnen, dann Build, Upload und Serial Monitor (115200). Auf der Kommandozeile:

```bash
pio run                 # bauen
pio run -t upload       # flashen
pio device monitor      # serielle Ausgabe ("GLT RLT demo ready")
tools/host_preview/build.sh   # optional: UI-Vorschau als PNG in .pio/host_preview/out
```

## Bekannte Einschränkungen

- Die Hardware wurde in dieser Version nicht getestet. Geprüft sind nur der Build (`pio run`) und die PC-Vorschau mit identischer LVGL-Konfiguration. Die Sichtprüfung auf dem Display steht noch aus.
- Die kleinste aktivierte Schrift ist Montserrat 14. Umlaute fehlen in den eingebauten Fonts, deshalb wird „ae/oe/ue“ geschrieben.
- Die Hydraulik (Heiz-/Kühl-/WRG-Kreis, „zum Verdichter !“) ist symbolisch nach der Referenzansicht gezeichnet. Daraus ist keine Funktion abgeleitet.
- Die Medienfarben sind eine Projektkonvention, keine Norm.
- Nicht enthalten: REST/MQTT/BACnet, Sollwert-Schreiben, Alarmquittierung, Trends, mehrere Anlagen.
