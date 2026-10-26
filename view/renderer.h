#pragma once

#include "view/camera.h"
#include "world/world.h"

#include <vector>

/** Scalar single-sample renderer for a triangle world. */
class Renderer {
public:
    /** Renders an image and updates optional intersection counters. */
    [[nodiscard]] static std::vector<Vector3> render(const World& world, const Camera& camera,
                                                      int width, int height, RenderStats* stats = nullptr,
                                                      TriangleIntersectionAlgorithm algorithm = TriangleIntersectionAlgorithm::Badouel);
};
