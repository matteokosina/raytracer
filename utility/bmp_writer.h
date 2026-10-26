#pragma once

#include "math/vector3.h"

#include <filesystem>
#include <vector>

/** Writes an RGB image buffer as an uncompressed 24-bit BMP. */
class BmpWriter {
public:
    /** Writes width*height colors, with row zero representing the top row.
     * @param path Destination filesystem path.
     * @param width Image width in pixels.
     * @param height Image height in pixels.
     * @param pixels Row-major linear RGB colors in [0,1].
     */
    static void write(const std::filesystem::path& path, int width, int height,
                      const std::vector<Vector3>& pixels);
};
