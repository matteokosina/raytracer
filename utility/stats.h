#pragma once

#include <atomic>
#include <cstdint>

/** Thread-safe counters used to compare renderer implementations. */
struct RenderStats {
    std::atomic<std::uint64_t> primary_rays{0};
    std::atomic<std::uint64_t> triangle_tests{0};
    std::atomic<std::uint64_t> aabb_tests{0};

    /** Resets all counters to zero. Call only when no render is running. */
    void reset() { primary_rays = 0; triangle_tests = 0; aabb_tests = 0; }
};
