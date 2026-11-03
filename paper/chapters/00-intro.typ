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
Der Raytracer ist in Module mit klar getrennten Aufgaben gegliedert. Die
Kommandozeile legt Eingabedatei, Bildgröße und Schnittalgorithmus fest.
Der `ObjLoader` lädt das OBJ-Modell samt Materialien und überführt seine
Flächen in Dreiecke, die das `World`-Objekt verwaltet. Die `Camera` erzeugt
für jedes Pixel einen Strahl; der `Renderer` verfolgt diese Strahlen und
berechnet die Beleuchtung. Für den nächsten Schnittpunkt fragt er die
`World` ab, die aktuell alle Dreiecke per Brute Force prüft. Dabei kann
`Triangle` entweder den Badouel- oder den Möller-Trumbore-Algorithmus
verwenden. Abschließend schreibt `BmpWriter` das berechnete Bild als BMP.

Die Trennung erleichtert den Vergleich der Schnittalgorithmen: Sie lassen
sich über die Kommandozeile via Flags auswählen, während Kamera, Szenenverwaltung
und Bildausgabe gleich bleiben. Geometrie, Beleuchtung und Dateiverarbeitung
sind dadurch als eigene Verantwortungsbereiche erkennbar.
#figure(
  image("../assets/class-diagram.svg", width: 100%),
  caption: [Klassendiagramm der zentralen Klassen. Rauten markieren enthaltene Objekte; gestrichelte Pfeile zeigen Nutzungsbeziehungen.],
)<fig:class-diagram>


Für die erste Aufgabe schauen wir uns hierzu zunächst zwei verschiedene Schnittpunktalgorithmen an. Diese sind der Badouel Algorithmus der als Referenzalgorithmus dient und der Möller-Trumbore Algorithmus. Standardmäßig nutzt der hier gebaute Raytracer den Badouel Algorithmus um Schnittpunkte zwischen der Dreiecken und den Strahlen zu berechnen, um den Möller-Trumbore Algorithmus zu nutzen muss die `--moeller` CLI-Flag mitgegeben werden.
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
Für den quantitativen Vergleich wurde die Szene mit 640 × 360 Pixeln
gerendert. Die Laufzeit stammt aus der vom Programm ausgegebenen
Wandzeitmessung und umfasst Szenenimport, Rendering und BMP-Ausgabe. Beide
Verfahren prüfen bei dieser Brute-Force-Szene dieselbe Anzahl an Dreiecken.

#table(
  columns: (1.1fr, 0.9fr, 1.1fr, 2.5fr),
  align: (left, right, right, left),
  [*Verfahren*], [*Laufzeit [ms]*], [*Dreieckstests [Anzahl]*], [*Profiling-Hotspots [Stichproben, macOS `sample`]*],
  [Badouel (Baseline)], [120619], [9647480640], [`Triangle::intersect`: 8966; `World::intersect`: 1915],
  [Möller–Trumbore (`--mt`)], [71737], [9647480640], [`Triangle::intersect`: 5448; `World::intersect`: 1025],
)

Die Hotspots in dieser Tabelle stammen aus macOS-`sample`-Funktionsstichproben;
die Zahlen sind Stichprobenzahlen, keine Aufrufzahlen. Die Laufzeiten stammen
aus denselben Profiling-Läufen. Möller–Trumbore war dabei rund 40,5 % schneller.

== Skalierung nach Bildauflösung
Zur Untersuchung der Skalierung wurden beide Verfahren mit Google Perftools
bei drei Auflösungen ausgeführt. Die Tabelle zeigt die gemessene Laufzeit und
das Verhältnis Badouel/Möller–Trumbore. Die Pixelzahl vervierfacht sich bei
jeder Verdopplung von Breite und Höhe; entsprechend wachsen die Laufzeiten
nahezu um den Faktor vier.

#table(
  columns: (1fr, 1fr, 1.2fr, 0.9fr, 1.5fr),
  align: (right, right, right, right, right),
  [*Auflösung [px]*], [*Badouel [ms]*], [*Möller–Trumbore [ms]*], [*Speedup [×]*], [*Dreieckstests [Anzahl, je Verfahren]*],
  [160 × 90], [7605], [4590], [1.66], [603621200],
  [320 × 180], [29343], [17359], [1.69], [2406027792],
  [640 × 360], [116764], [70305], [1.66], [9647480640],
)

Der Speedup bleibt über die drei Bildgrößen stabil bei 1,66–1,69. Beim
Verdoppeln beider Bilddimensionen steigt die Laufzeit zwischen 3,78× und 4,05×;
das entspricht näherungsweise einer linearen Skalierung mit der Pixelzahl.
Die Perftools-Profile wurden je Verfahren und Auflösung erzeugt. Die
640 × 360-Callgraphs zeigen `Triangle::intersect` als dominanten Hotspot mit
80,15 % der Badouel- beziehungsweise 82,16 % der Möller–Trumbore-Stichproben.

Die folgenden beiden Abschnitte erläutern die Funktionsweise der beiden Schnittpunktalgorithmen.
== Badouel Algorithmus
Seien $A$, $B$ und $C$ die Dreiecksecken sowie $O + t D$ der Strahl. Badouel
bildet zunächst $e_1 = B - A$, $e_2 = C - A$ und die Flächennormale
$N = e_1 times e_2$. Ist $N dot N$ nahezu null, ist das Dreieck degeneriert.
Ansonsten schneidet der Algorithmus den Strahl mit der Dreiecksebene:
$t = (N dot (A - O)) / (N dot D)$. Ein nahezu verschwindender Nenner bedeutet,
dass der Strahl parallel zur Ebene verläuft; zusätzlich muss $t$ im erlaubten
Strahlintervall liegen.

Für den Innen-Test wird die Koordinate mit dem größten Betrag in $N$
weggelassen. Dadurch wird das Dreieck auf diejenige Koordinatenebene projiziert,
auf der seine Fläche am größten erscheint. Aus den 2D-Koordinaten berechnet der
Algorithmus die baryzentrischen Gewichte $u$ und $v$. Der Treffer liegt im
Dreieck, wenn $u >= 0$, $v >= 0$ und $u + v <= 1$ gilt. Der Code verwirft zudem
eine nahezu verschwindende projizierte Fläche.

== Möller-Trumbore Algorithmus
Möller–Trumbore berechnet die baryzentrischen Koordinaten direkt in 3D und
benötigt keine Projektion. Mit $e_1 = B - A$, $e_2 = C - A$ und
$P = D times e_2$ ergibt sich $det = e_1 dot P$. Ist $|det|$ nahezu null,
verläuft der Strahl parallel zur Dreiecksebene oder das Dreieck ist degeneriert.
Andernfalls setzt der Code $T = O - A$ und berechnet
$u = (T dot P) / det$. Werte außerhalb $0 <= u <= 1$ werden sofort verworfen.

Für den zweiten baryzentrischen Parameter bildet der Algorithmus
$Q = T times e_1$ und berechnet $v = (D dot Q) / det$. Es gilt dieselbe
Innenbedingung $v >= 0$ und $u + v <= 1$. Die Strahlposition folgt aus
$t = (e_2 dot Q) / det$ und wird gegen das vorgegebene Intervall geprüft.
Der Test ist zweiseitig: Durch den Betrag des Determinanten wird keine
Dreiecksorientierung als Rückseiten-Culling verwendet.

== Mikrooperationen bis zur ersten u-Prüfung
Die folgende Zählung verwendet die Operationen im aktuellen C++-Quelltext und
endet jeweils unmittelbar nach der Berechnung von $u$, direkt vor dem Test
$0 <= u <= 1$. Vektoradditionen, Subtraktionen und Vergleiche sind nicht als
Multiplikationen oder Divisionen gezählt. Es wird keine konstante Division
angenommen.

Bei Badouel entstehen 6 Multiplikationen für $n = e_1 times e_2$, weitere 3
für den Degeneriertheitstest $n dot n$, je 3 für $n dot D$ und
$n dot (A - O)$, 3 für den Trefferpunkt $O + t D$, 2 für die projizierte
Dreiecksfläche und 2 für den Zähler von $u$. Das ergibt 22 Multiplikationen.
Die Divisionen zur Berechnung von $t$ und $u$ ergeben zusammen 2. Die oft
angegebene Zahl 19 lässt die 3 Multiplikationen für $n dot n$ aus; dieser
Degeneriertheitstest ist aber im aktuellen Code enthalten.

Bei Möller–Trumbore benötigt $P = D times e_2$ 6 Multiplikationen,
$e_1 dot P$ und $T dot P$ je 3 sowie die Multiplikation mit $1 / det$ eine
weitere. Damit sind es 13 Multiplikationen und eine Division zur Bildung des
Reziproken $1 / det$.

#table(
  columns: (1.4fr, 1.3fr, 1.2fr, 2.6fr),
  align: (left, right, right, left),
  [*Verfahren*], [*Multiplikationen [Anzahl]*], [*Divisionen [Anzahl]*], [*Bedingte Prüfungen bis einschließlich u-Test [Anzahl]*],
  [Badouel], [22], [2], [14–18; abhängig von Projektionsachse],
  [Möller–Trumbore], [13], [1], [4 im C++-Quelltext],
)

Die Zählung umfasst 8 Prüfungen außerhalb der
Achsenwahl, 2 Vergleiche bei der Achsenwahl sowie 4–8 Tests beim Abbilden der
vier Dreieck-/Trefferpunkte auf 2D-Koordinaten. Möller–Trumbore hat bis zum
u-Test eine Algorithmusauswahl, einen Determinantentest und zwei
Kurzschlussvergleiche für die u-Grenzen.

= Build- & Profiling-Infrastruktur
Aufgrund von Kompatibilitätsbeschränkungen klassischer GCC-Profiling-Tools
(gprof) auf der macOS-Plattform wurden die dynamischen Laufzeit- und
Hotspot-Analysen mittels Google Performance Tools (gperftools / pprof)
durchgeführt.

=== CPU-Profiling mit Google Perftools
Das Skript `scripts/profile_task1_gperftools.sh` baut mit
`-DENABLE_GPERFTOOLS_PROFILING=ON`, rendert beide Verfahren und analysiert die
CPU-Profile mit `pprof`. Die folgende Tabelle zeigt die Wandzeit aus dem
Raytracer-Log sowie die von `pprof` aus Stichproben geschätzte CPU-Zeit. Die
mittlere Zeit pro Test ist die `Triangle::intersect`-Eigenzeit geteilt durch
die vom Renderer gezählte Anzahl der Dreieckstests; sie ist daher ebenfalls
eine Stichprobenschätzung, keine gemessene Einzelaufrufdauer.

#table(
  columns: (1.2fr, 1fr, 1.2fr, 1.2fr, 0.9fr, 1fr),
  align: (left, right, right, right, right, right),
  [*Verfahren*], [*Wandzeit [ms]*], [*Profilzeit [s]*], [*Triangle-Zeit [s]*], [*Anteil [%]*], [*Ø je Test [ns]*],
  [Badouel], [116764], [93.07], [74.60], [80.15], [7.733],
  [Möller–Trumbore], [70305], [56.85], [46.71], [82.16], [4.842],
)

Die mittlere Testzeit ist eine Stichprobenschätzung: Sie teilt die
`Triangle::intersect`-Eigenzeit durch die vom Renderer gezählten Tests. Der
Profiler liefert damit keine Einzelaufrufmessung.
