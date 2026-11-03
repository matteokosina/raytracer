#pragma once

#include "view/camera.h"
#include "world/world.h"

#include <functional>
#include <vector>

/** Scalar single-sample renderer for a triangle world. */
class Renderer {
public:
    using ProgressCallback = std::function<void(int)>;

    /** Renders an image, updates counters, and reports integer progress percentages.
     * @param world Scene whose triangles are traced.
     * @param camera Camera that generates primary rays.
     * @param width Image width in pixels.
     * @param height Image height in pixels.
     * @param stats Optional render counters; may be null.
     * @param intersect_fn Triangle intersection implementation selected before rendering.
     * @param progress Optional callback called when the completed-row percentage changes.
     * @return Row-major linear RGB pixels in [0,1].
     */
    [[nodiscard]] static std::vector<Vector3> render(const World& world, const Camera& camera,
                                                      int width, int height, RenderStats* stats = nullptr,
                                                      TriangleIntersectFn intersect_fn = &Triangle::intersect_badouel,
                                                      ProgressCallback progress = {});
};
