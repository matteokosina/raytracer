#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
OUTPUT_DIR="${OUTPUT_DIR:-${BUILD_DIR}/profiles/task1-gperftools}"
WIDTH="${WIDTH:-640}"
HEIGHT="${HEIGHT:-360}"

if [[ -n "${PPROF:-}" ]]; then
    PPROF_CMD="${PPROF}"
elif command -v pprof >/dev/null 2>&1; then
    PPROF_CMD="$(command -v pprof)"
elif command -v go >/dev/null 2>&1 && [[ -x "$(go env GOPATH)/bin/pprof" ]]; then
    PPROF_CMD="$(go env GOPATH)/bin/pprof"
else
    echo "error: pprof was not found; install github.com/google/pprof or set PPROF=/path/to/pprof." >&2
    exit 2
fi

if [[ -f "${ROOT_DIR}/scenes/teapot_n_glass.obj" ]]; then
    SCENE="${ROOT_DIR}/scenes/teapot_n_glass.obj"
else
    SCENE="${ROOT_DIR}/teapot_n_glass.obj"
fi
if [[ ! -f "${SCENE}" ]]; then
    echo "error: teapot_n_glass.obj was not found in scenes/ or the project root." >&2
    exit 2
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DENABLE_GPERFTOOLS_PROFILING=ON
cmake --build "${BUILD_DIR}" --target raytracer -j
mkdir -p "${OUTPUT_DIR}"

run_profile() {
    local name="$1"
    shift
    local profile="${OUTPUT_DIR}/${name}.prof"
    local log="${OUTPUT_DIR}/${name}.log"
    local report="${OUTPUT_DIR}/${name}.pprof.txt"

    CPUPROFILE="${profile}" "${BUILD_DIR}/raytracer" "$@" \
        --input "${SCENE}" -w "${WIDTH}" -h "${HEIGHT}" \
        -o "${OUTPUT_DIR}/${name}.bmp" > "${log}" 2>&1
    "${PPROF_CMD}" --text --nodecount=20 "${BUILD_DIR}/raytracer" \
        "${profile}" > "${report}" 2> "${OUTPUT_DIR}/${name}.pprof.stderr"
}

run_profile badouel
run_profile moeller --mt

python3 "${ROOT_DIR}/scripts/analyze_perftools_task1.py" \
    "${OUTPUT_DIR}/badouel.pprof.txt" "${OUTPUT_DIR}/badouel.log" \
    "${OUTPUT_DIR}/moeller.pprof.txt" "${OUTPUT_DIR}/moeller.log" \
    > "${OUTPUT_DIR}/summary.csv"

echo "Profiles, pprof reports, and summary written to ${OUTPUT_DIR}"
