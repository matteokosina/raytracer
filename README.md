# C++20 Raytracer

A small modular CPU raytracer that loads Wavefront OBJ meshes, traces primary
rays against triangles, applies simple diffuse lighting, and writes a 24-bit BMP.
The default is the scalar Badouel projection baseline from `AGENTS.md`;
Möller–Trumbore and median-split k-d tree acceleration are optional modes
behind the same scene and renderer interfaces.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
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
./build/raytracer --kdtree --mt -w 320 -h 180 -o teapot-kdtree.bmp
```

Options: `-i`/`--input` selects an OBJ file, `-w` and `-h` set the pixel
dimensions, and `-o` selects the BMP destination. Polygon faces are fan
triangulated, negative OBJ vertex indices are supported, and diffuse `Kd`
colors, specular `Ks`, shininess `Ns`, opacity `d`, and transparency `Tr` are
read from referenced MTL files. OBJ vertex normals are interpolated across
triangle surfaces for smooth shading. Specular materials trace reflected rays,
and transparent materials trace refracted rays using `Ni` and Fresnel weighting.
Direct illumination includes hard shadow rays, with transparent surfaces
attenuating shadows. The glass model is a compact Whitted-style approximation,
not a full physically based renderer. Traversal defaults to brute force;
`--kdtree` builds the acceleration tree once after scene loading and uses it
for primary, shadow, reflected and refracted rays.

Triangle intersection defaults to Badouel. Pass `--moeller` or `--mt` to select
Möller–Trumbore; both options use the same scene traversal and shading code.

The tree partitions triangle indices at the median centroid, cycling X/Y/Z
axes, with at most six triangles per leaf or twenty subdivision levels.
Child boxes enclose complete triangles and may overlap; traversal visits the
origin-side child first and prunes only with closed AABB slab intervals.
Parallel directions, tiny nonzero directions and zero-width boxes are supported.
Equal-distance hits select the largest original triangle index in both modes.
Rendering selects a compile-time traversal specialization before the pixel loop,
so there is no per-ray traversal-mode branch. Output reports atomic triangle
and AABB test counts for the entire frame, including secondary rays.

`World::build_kdtree()` retains a move-safe geometry snapshot. Adding a triangle
invalidates the tree; rebuild it after mutations to enable acceleration again.

## Modules

`math` contains vector operations; `geometry` defines rays and triangle
intersections; `world` owns scene triangles and closest-hit queries; `view`
creates camera rays and renders pixels; `io` handles OBJ/MTL input, BMP
output, and counters. All scene and image data use value semantics.
