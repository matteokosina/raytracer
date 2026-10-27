BUILD_DIR ?= build
BUILD_TYPE ?= Release
WIDTH ?= 3840
HEIGHT ?= 2160
ASSET_DIR ?= paper/assets

BADOUEL_BMP ?= teapot-badouel.bmp
MOELLER_BMP ?= teapot-moeller.bmp
BMP = $(BADOUEL_BMP)
PNG_NAME = $(basename $(notdir $(BMP))).png

.PHONY: all help build run-badouel run-moeller run-both convert-bmp \
	convert-badouel convert-moeller assets clean

all: build

help:
	@printf '%s\n' \
	  'make build                  Configure and build with CMake' \
	  'make run-badouel            Render using Badouel' \
	  'make run-moeller            Render using Moller-Trumbore' \
	  'make run-both               Render using both algorithms' \
	  'make convert-bmp BMP=file.bmp [PNG_NAME=file.png]' \
	  'make assets                 Render both modes and put PNGs in paper/assets' \
	  'Set WIDTH and HEIGHT to change the render resolution.'

build:
	cmake -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)"
	cmake --build "$(BUILD_DIR)"

run-badouel: build
	"$(BUILD_DIR)/raytracer" -w "$(WIDTH)" -h "$(HEIGHT)" -o "$(BADOUEL_BMP)"

run-moeller: build
	"$(BUILD_DIR)/raytracer" --moeller -w "$(WIDTH)" -h "$(HEIGHT)" -o "$(MOELLER_BMP)"

run-both: run-badouel run-moeller

convert-bmp:
	@test -n "$(BMP)" || { echo 'Set BMP to the source .bmp file, for example: make convert-bmp BMP=teapot-badouel.bmp'; exit 2; }
	@test -f "$(BMP)" || { echo 'BMP file not found: $(BMP)'; exit 2; }
	@command -v sips >/dev/null 2>&1 || { echo 'sips is required (it is included with macOS).'; exit 2; }
	@mkdir -p "$(ASSET_DIR)" "$(BUILD_DIR)/converted"
	sips -s format png "$(BMP)" --out "$(BUILD_DIR)/converted/$(PNG_NAME)" >/dev/null
	mv "$(BUILD_DIR)/converted/$(PNG_NAME)" "$(ASSET_DIR)/$(PNG_NAME)"

convert-badouel: run-badouel
	$(MAKE) --no-print-directory convert-bmp BMP="$(BADOUEL_BMP)" PNG_NAME=teapot-badouel.png

convert-moeller: run-moeller
	$(MAKE) --no-print-directory convert-bmp BMP="$(MOELLER_BMP)" PNG_NAME=teapot-moeller.png

assets: run-both
	$(MAKE) --no-print-directory convert-bmp BMP="$(BADOUEL_BMP)" PNG_NAME=teapot-badouel.png
	$(MAKE) --no-print-directory convert-bmp BMP="$(MOELLER_BMP)" PNG_NAME=teapot-moeller.png

clean:
	cmake -E rm -rf "$(BUILD_DIR)"
