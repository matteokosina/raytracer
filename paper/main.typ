#import "template.typ": *

#show: project.with(
  title: "Projektbericht",
  work-type: "",
  lecture: "Labor Algorithmen / Optimierung von Programmen",
  professor: "Prof. Dr. rer. nat. Christian Pape ",
  authors: (
    (name: "Matteo Kosina"),
  ),
)

#include "chapters/00-intro.typ"
#pagebreak()
#bibliography("references.bib", title: [Literatur und Bildquellen], style: "ieee")
#include "appendix.typ"

