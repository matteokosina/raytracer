# C++20 Raytracer

A small modular CPU raytracer that loads Wavefront OBJ meshes, traces primary
rays against triangles, applies simple diffuse lighting, and writes a 24-bit BMP.
The initial implementation is the scalar Badouel projection baseline from
`AGENTS.md`; later intersection and acceleration variants can be added behind
the same scene and renderer interfaces.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

GoogleTest is optional and is detected if already installed. No dependency is
downloaded during configuration. To build without tests, pass
`-DBUILD_TESTING=OFF`. Profiling and coverage builds are available through
`-DENABLE_PROFILING=ON` and `-DENABLE_COVERAGE=ON`.

## Render

Run from the project directory so the default OBJ path resolves:

```sh
./build/raytracer
./build/raytracer --input teapot_n_glass.obj -w 320 -h 180 -o teapot.bmp
./build/raytracer --mt --input teapot_n_glass.obj -w 320 -h 180 -o teapot-mt.bmp
```

Options: `-i`/`--input` selects an OBJ file, `-w` and `-h` set the pixel
dimensions, and `-o` selects the BMP destination. Polygon faces are fan
triangulated, negative OBJ vertex indices are supported, and diffuse `Kd`
colors, specular `Ks`, shininess `Ns`, opacity `d`, and transparency `Tr` are
read from referenced MTL files. OBJ vertex normals are interpolated across
triangle surfaces for smooth shading. Specular materials trace reflected rays,
and transparent materials trace refracted rays using `Ni` and Fresnel weighting.
The glass model is a compact Whitted-style approximation, not a full physically
based renderer. The current renderer is brute force;
large meshes at full resolution are intentionally a slow baseline until the
k-d tree phase is implemented.

Triangle intersection defaults to Badouel. Pass `--moeller` or `--mt` to select
Möller–Trumbore; both options use the same scene traversal and shading code.

## Modules

`math` contains vector operations; `geometry` defines rays and triangle
intersections; `world` owns scene triangles and closest-hit queries; `view`
creates camera rays and renders pixels; `utility` handles OBJ/MTL input, BMP
output, and counters. All scene and image data use value semantics.
