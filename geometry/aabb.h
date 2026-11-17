#pragma once

#include "geometry/triangle.h"

/** Closed axis-aligned box, including zero-width slabs. */
class AABB {
public:
    Vector3 min_bounds; ///< Component-wise lower corner in world units.
    Vector3 max_bounds; ///< Component-wise upper corner in world units.

    /** Tests the intersection of three closed ray/slab intervals.
     * @param ray World-space ray; direction need not be normalized.
     * @param t_min Lower ray parameter (world units for a unit direction).
     * @param t_max Upper ray parameter, with the same units as t_min.
     * @return True if the box overlaps [t_min,t_max], including tangency.
     * @pre Bounds are finite and component-wise ordered; ray components are finite.
     * @post Neither the ray nor the box is changed. Reversed intervals miss.
     * Exact zero directions use slab containment; tiny nonzero directions retain
     * their intersections without overflowing a reciprocal or computing 0*infinity.
     */
    [[nodiscard]] bool intersect(const Ray& ray, double t_min, double t_max) const;

    /** Computes the component-wise bounds of all three vertices.
     * @param tri Triangle with finite world-space vertices.
     * @return Smallest closed box enclosing tri, in world units.
     * @post Degenerate and axis-aligned triangles remain enclosed.
     */
    [[nodiscard]] static AABB from_triangle(const Triangle& tri);

    /** Merges two boxes without modifying either input.
     * @param box1 First finite, ordered world-space box.
     * @param box2 Second finite, ordered world-space box.
     * @return Smallest closed world-space box enclosing both inputs.
     */
    [[nodiscard]] static AABB surround(const AABB& box1, const AABB& box2);
};
