#!/usr/bin/env python3
"""Extract sampled CPU time and renderer counts from pprof text reports."""

import csv
import re
import sys
from pathlib import Path


TOTAL = re.compile(r"of\s+([\d.]+)s\s+total")
TRIANGLE = re.compile(r"^\s*([\d.]+)s\s+([\d.]+)%.*\sTriangle::intersect\s*$", re.M)
TESTS = re.compile(r"triangle tests:\s*(\d+)", re.IGNORECASE)


def extract(report: Path, log: Path) -> tuple[float, float, float, int, float]:
    text = report.read_text(errors="replace")
    total_match = TOTAL.search(text)
    triangle_match = TRIANGLE.search(text)
    test_match = TESTS.search(log.read_text(errors="replace"))
    if not total_match or not triangle_match or not test_match:
        raise ValueError(f"missing total, Triangle::intersect, or triangle count in {report} / {log}")
    total_seconds = float(total_match.group(1))
    triangle_seconds = float(triangle_match.group(1))
    percent = float(triangle_match.group(2))
    test_count = int(test_match.group(1))
    ns_per_test = triangle_seconds * 1e9 / test_count
    return total_seconds, triangle_seconds, percent, test_count, ns_per_test


def main() -> int:
    if len(sys.argv) != 5:
        print("usage: analyze_perftools_task1.py BADOUEL.pprof BADOUEL.log MOELLER.pprof MOELLER.log",
              file=sys.stderr)
        return 2
    writer = csv.writer(sys.stdout)
    writer.writerow(("algorithm", "profiled_cpu_seconds", "triangle_self_seconds",
                     "triangle_percent", "triangle_tests", "ns_per_test"))
    try:
        for label, report, log in (
            ("Badouel", Path(sys.argv[1]), Path(sys.argv[2])),
            ("Moller-Trumbore", Path(sys.argv[3]), Path(sys.argv[4])),
        ):
            total, triangle, percent, tests, ns = extract(report, log)
            writer.writerow((label, f"{total:.2f}", f"{triangle:.2f}",
                             f"{percent:.2f}", tests, f"{ns:.3f}"))
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
