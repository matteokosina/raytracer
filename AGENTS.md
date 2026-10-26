# AGENTS.md — Entwicklungsanweisung für den C++ Raytracer (Optimierung von Programmen)

Diese Datei dient als verbindliche Richtlinie und Prompt-Instruktion für KI-Agenten (Claude, Cursor, Gemini CLI) zur schrittweisen Implementierung des C++20 Raytracers.

---

## 🎯 Projektziel & Rahmenbedingungen
* **Vorlesung:** Optimierung von Programmen (Pass/Fail)
* **Sprache:** C++20
* **Build-System:** CMake (>= 3.20) mit GCC/Clang
* **Architektur-Motto:** EIN einziger modularer Raytracer. Keine getrennten Programme für Optimierungen, sondern Steuerung aller Varianten über CLI-Flags.
* **Verantwortung:** Der Entwickler muss jede Zeile Code in der mündlichen Abgabe erklären können.

---

## 📐 Architektur- & Modul-Regeln (LLM Coding Guidelines)
1. **Modulgröße:** Maximal 300–400 Zeilen pro Datei.
2. **Trennung Interface & Logik:**
   * Header-Dateien (`.h`): Enthält Klassendeklarationen, Methodensignaturen und vollständige **Doxygen-Dokumentation** (Parameter mit Einheiten, Rückgabewerte, Vor-/Nachbedingungen).
   * Implementierung (`.cc`): Reine Logik ohne/mit minimalen Inline-Kommentaren.
3. **Speicherverwaltung & Performance:**
   * RAII & Value Semantics bevorzugen (`std::vector`, `std::unique_ptr`).
   * Übergabe komplexer Typen als `const T&`.
   * Alignment für SIMD berücksichtigen (`alignas(32)`).
4. **Git Workflow:** Nach jedem funktionsfähigen Teilmodul ein sauberer Git-Commit (`feat: ...`, `test: ...`).

---

## 📂 Projektstruktur

```text
raytracer/
├── CMakeLists.txt              # Zentrales Build-Skript (App, GTest, Profiling)
├── raytracer.cc                # Main: Parst CLI-Flags & startet Pipeline
├── math/                       # Vektormathematik & Zufall
│   ├── vector3.h / .cc
│   └── random.h / .cc
├── geometry/                   # Geometrische Primitiven & Tests
│   ├── ray.h
│   ├── triangle.h / .cc        # Badouel & Möller-Trumbore (Aufgabe 1)
│   ├── aabb.h / .cc            # Bounding Box (Slab-Test) (Aufgabe 2)
│   └── kdtree.h / .cc          # k-d-Baum mit Median-Split (Aufgabe 2)
├── world/                      # Szenenverwaltung
│   ├── world.h / .cc           # Hält Dreiecke, steuert Brute-Force vs. k-d-Baum
│   └── material.h
├── view/                       # Kamera & Ausführung
│   ├── camera.h / .cc          # Strahl-Erzeugung (Skalar & AVX Packet Tracing)
│   └── renderer.h / .cc        # Multithreading / Rendering-Schleife
├── utility/                    # Mess-Logik & I/O
│   ├── timer.h                 # std::chrono Messung
│   ├── stats.h / .cc           # Atomic-Zähler für Schnittpunkttests
│   ├── obj_loader.h / .cc      # Wavefront .obj Import
│   └── bmp_writer.h / .cc      # BMP-Export
└── tests/                      # GoogleTest Unit-Tests
    ├── CMakeLists.txt
    ├── math_test.cc
    └── geometry_test.cc
```

---

## ⚙️ CLI-Flags & Modi (Steuerung)

Das Hauptprogramm `raytracer` akzeptiert folgende Steuerungs-Flags:

| Flag | Standard | Auswirkung | Rechneraufgabe |
| :--- | :--- | :--- | :--- |
| *(kein Flag)* | **Badouel** + Brute-Force | Baseline Raytracer aus der Vorlesung | Aufgabe 1 (Baseline) |
| `--moeller` oder `--mt` | deaktiviert | Nutzen des Möller-Trumbore Schnittpunkttests | Aufgabe 1 (Optimierung) |
| `--kdtree` | deaktiviert | Nutzen der k-d-Baum Beschleunigungsstruktur | Aufgabe 2 (Optimierung) |
| `--avx` | deaktiviert | Nutzen des AVX2 Packet Tracing (4–8 Strahlen) | Aufgabe 3 (Optimierung) |
| `-w <px>` / `-h <px>` | 1280x720 | Bildauflösung festlegen | — |
| `-o <datei.bmp>` | raytracer.bmp | Pfad der Ausgabedatei | — |

---

## 🛠️ Ausführungsplan für Agenten (Phase für Phase)

### Phase 1: Foundation & Baseline (Aufgabe 1 - Teil 1)
- [ ] Setup `CMakeLists.txt` mit C++20, GoogleTest-Integration und Profiling-Optionen (`ENABLE_PROFILING` für `gprof`, `ENABLE_COVERAGE` für `gcov`).
- [ ] Implementierung `math/vector3.h` (mit korrigiertem Kreuzprodukt!).
- [ ] Implementierung `utility/stats.h` mit atomic Zählern (`primary_rays`, `triangle_tests`, `aabb_tests`).
- [ ] Implementierung `geometry/triangle.cc` mit **Badouel-Schnittpunkttest** (Projektion auf 2D, $u, v$ Parameter).
- [ ] Implementierung `world/world.cc` (Brute-Force über alle Dreiecke) und `utility/bmp_writer.cc`.
- [ ] GoogleTest `geometry_test.cc` zur Validierung des Badouel-Tests gegen bekannte Geometrie.

### Phase 2: Möller-Trumbore Optimierung (Aufgabe 1 - Teil 2)
- [ ] Ergänzung `Triangle::intersect()` um den **Möller-Trumbore-Algorithmus** (Cramer'sche Regel, direkt in 3D).
- [ ] Einbau CLI-Flag `--moeller` / `--mt` in `raytracer.cc`.
- [ ] Ergänzung GoogleTest Äquivalenz-Test: 20.000 zufällige Strahlen müssen bei Badouel und Möller-Trumbore **exakt dieselben Hits/Misses, t, u, v** liefern.
- [ ] Profiling-Lauf mit `gprof` dokumentieren (Badouel vs. Möller-Trumbore).

### Phase 3: k-d-Baum Beschleunigungsstruktur (Aufgabe 2)
- [ ] Implementierung `geometry/aabb.h` mit **Slab-Test** (Strahl vs. AABB).
- [ ] Implementierung `geometry/kdtree.cc`:
  - Rekursiver Aufbau mit Unterteilung am **Median** der Dreiecksmittelpunkte.
  - Achsenwechsel $X \to Y \to Z \to X$.
  - Effiziente Traversierung: Nur Äste durchsuchen, wenn der AABB-Slab-Test positiv ist.
- [ ] Einbau CLI-Flag `--kdtree` in `raytracer.cc`.
- [ ] GoogleTest Äquivalenz-Test: Brute-Force vs. k-d-Baum müssen bei komplexen Szenen (`.obj`) das identische vorderste Dreieck finden.
- [ ] Vergleich der `triangle_intersection_tests` im Console-Output (Brute-Force vs. k-d-Baum).

### Phase 4: AVX2 Packet Tracing (Aufgabe 3)
- [ ] Speicherausrichtung: `alignas(32)` für Strahlpakete und Vektoren sicherstellen.
- [ ] Bündelung von 4–8 kohärenten Primärstrahlen in der Camera/Renderer-Schleife.
- [ ] Implementierung parallelisierter Slab-Test & Möller-Trumbore-Test mit **AVX2-Intrinsics** (`<immintrin.h>`) oder GCC-Vektortypen.
- [ ] Einbau CLI-Flag `--avx` in `raytracer.cc`.
- [ ] Benchmark: Skalare Ausführung vs. AVX Packet Tracing messen.

