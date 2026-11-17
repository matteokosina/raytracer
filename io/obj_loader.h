#pragma once

#include "world/world.h"

#include <filesystem>

/** Reads vertex positions and polygon faces from a Wavefront OBJ file. */
class ObjLoader {
public:
    /** Loads an OBJ, triangulating polygon faces as a fan.
     * @param path Filesystem path to the OBJ file.
     * @return A world containing the loaded triangles.
     * @throws std::runtime_error when the file cannot be read or is malformed.
     */
    [[nodiscard]] static World load(const std::filesystem::path& path);
};
