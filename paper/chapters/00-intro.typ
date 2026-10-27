= Einleitung und Zielsetzung
Die Erzeugung dreidimensionaler Szenen mittels Raytracing gehört zu den grundlegenden Verfahren der Computergrafik. Im Rahmen der Vorlesung *Optimierung von Programmen* steht die schrittweise Performanzsteigerung eines in C++ umgesetzten Whitted-Style Raytracers im Vordergrund. Das physikalisch motivierte Prinzip des Raytracings simuliert den Weg des Lichts in umgekehrter Richtung: Von der Kamera aus werden für jedes einzelne Bildschirmpixel in der Bildfläche Sehstrahlen in die dreidimensionale Welt ausgesendet, um die Farbwerte anhand der getroffenen Objekte, Lichtquellen und Materialeigenschaften zu berechnen (siehe @fig:raytracing-schema).

Obwohl dieses Verfahren eine hohe visuelle Realitätsnähe mit exakten Schattenwurf-, Reflexions- und Brechungseffekten ermöglicht, bringt es einen enormen Rechenaufwand mit sich. Da für jedes Pixel und jeden rekursiven Folgestrahl potentiell Tausende bis Millionen von geometrischen Objekten auf einen Schnittpunkt geprüft werden müssen, bildet die Strahl-Dreieck-Schnittpunktberechnung den zentralen Flaschenhals der Anwendung.

#figure(
    image("../assets/raytracing.png", width: 100%),
    caption: [Schematische Darstellung des Raytracings. Adoptiert von @raytracing_diagram],
)<fig:raytracing-schema>

Ziel dieser Arbeit ist es, die Ausführungszeit der Bildsynthese durch systematische Optimierungen auf verschiedenen Systemebenen drastisch zu senken. 

Der vorliegende Projektbericht dokumentiert die Umsetzung und die erzielten Ergebnisse entlang der drei bearbeiteten Rechneraufgaben:

Zunächst behandelt das erste Kapitel die mathematische Optimierung der Strahl-Dreieck-Schnittpunktberechnung. Hierbei wird der klassische Badouel-Algorithmus als unoptimierte Ausgangslage dem Möller-Trumbore-Verfahren gegenübergestellt, welches ohne explizite Ebenengleichungen und 2D-Projektionen auskommt.

Das darauffolgende Kapitel widmet sich der algorithmischen Beschleunigung der Szenentraversierung. Durch den Aufbau eines hierarchischen k-d-Baums mit Median-Unterteilung und dem Einsatz von Axis-Aligned Bounding Boxes (AABB) in Kombination mit dem Slab-Test wird die Schnittpunktprüfung von einer linearen Brute-Force-Suche der Komplexität $O(N)$ auf eine logarithmische Suche der Komplexität $O(log N)$ reduziert.

Abschließend beschreibt das vierte Kapitel die hardwarenahe Optimierung mittels SIMD-Vektorisierung. Durch die Bündelung kohärenter Sehstrahlen zu Strahlpaketen werden die Berechnungen unter Ausnutzung von x86 AVX2-Instruktionen auf 256-Bit-Vektorregistern parallelisiert.

= Erste Aufgabe
TODO: Projekt overview + klassendiagramm


Um das Gesamtsystem vergleichbar zu halten, sind alle drei Optimierungsstufen über Kommandozeilen-Flags in einer einzigen ausführbaren Datei integriert. Für die erste Aufgabe schauen wir uns hierzu zunächst zwei verschiedene Schnittpunktalgorithmen an. Diese sind der Badouel Algorithmus der als Referenzalgorithmus dient und der Möller-Trumbore Algorithmus. Standardmäßig nutzt der hier gebaute Raytracer den Badouel Algorithmus um Schnittpunkte zwischen der Dreiecken und den Strahlen zu berechnen, um den Möller-Trumbore Algorithmus zu nutzen muss die `--moeller` CLI-Flag mitgegeben werden.
Als Testszene dient die Teapot-Szene die als _.obj_ und _.mtl_ Datei bereitgestellt wurden. @fig:rendered0 und @fig:rendered1 zeigen die vom Raytracer gerenderte Szene mittels Badouel und Möller-Trumbore Algorithmus in einer Auflösung von 3840px x 2160px. Wie erwartet zeigen Sie das gleiche Bild, da ausschlieplich die Berechnung des Schnittpunktes jedoch nicht das Ergebnis unterschiedlich sein sollte.
#grid(
  columns: (1fr, 1fr),
  column-gutter: 10pt,
  row-gutter: 10pt,
  [
  #figure(
    image("../assets/teapot-badouel.png", width: 100%),
    caption: [Teapot-Szene Badouel Algorithmus (3840x2160)],
  )<fig:rendered0>
  ],
  [
  #figure(
    image("../assets/teapot-moeller.png", width: 100%),
    caption: [Teapot-Szene Möller-Trumbore Algorithmus (3840x2160)],
  )<fig:rendered1>
  ]
)
Die folgenden beiden Abschnitte erläutern die Funktionsweise der beiden Schnittpunktalgorithmen.
== Badouel Algorithmus
== Möller-Trumbore Algorithmus
