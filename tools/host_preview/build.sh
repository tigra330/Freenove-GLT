#!/bin/bash
# Rendert die UI auf dem PC (gcc, gleiche LVGL-Version/lv_conf.h) als PNG-Dateien.
# Nur zur Layoutpruefung; ersetzt keinen Test auf dem echten Display.
# Aufruf aus dem Projektordner: tools/host_preview/build.sh  (vorher einmal "pio run")
set -e
cd "$(dirname "$0")/../.."
L=.pio/libdeps/freenove_5inch/lvgl
B=.pio/host_preview
mkdir -p "$B/obj" "$B/out"
find "$L/src" -name '*.c' | while read -r f; do
  o="$B/obj/$(echo "${f#$L/}" | tr / _).o"
  [ "$o" -nt "$f" ] || echo "gcc -c -O1 -w -DLV_CONF_INCLUDE_SIMPLE -Iinclude -I$L $f -o $o"
done | xargs -P"$(nproc)" -I{} sh -c '{}'
g++ -O1 -g -std=gnu++17 -DLV_CONF_INCLUDE_SIMPLE -Iinclude -I"$L" -Isrc -Ilib/lvgl-glt/src \
  tools/host_preview/preview.cpp src/ui/*.cpp src/model/*.cpp lib/lvgl-glt/src/*.cpp "$B"/obj/*.o -lm -o "$B/preview"
"$B/preview" "$B/out"
echo "Bilder in $B/out"
