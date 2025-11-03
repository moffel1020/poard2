#pragma once
#include <cmath>
#include <glm/glm.hpp>

namespace {
glm::vec2 randomGradient(int ix, int iy) {
    // idk how this works
    const uint32_t w = 8 * sizeof(uint32_t);
    const uint32_t s = w / 2;
    uint32_t a = ix;
    uint32_t b = iy;

    a *= 3284157443;

    b ^= a << s | a >> (w - s);
    b *= 1911520717;

    a ^= b << s | b >> (w - s);
    a *= 2048419325;
    const float random = a * (3.14159265 / ~(~0u >> 1)); // in 0 - 2pi

    return glm::vec2(std::sin(random), std::cos(random));
}

// computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    const auto gradient = randomGradient(ix, iy);
    const auto distance = glm::vec2(x - static_cast<float>(ix), y - static_cast<float>(iy));
    return glm::dot(distance, gradient);
}

constexpr float cubicInterp(float a0, float a1, float w) { return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0; }

} // anonymous namespace

namespace Noise {

inline float perlin(float x, float y) {
    // corners
    const int x0 = static_cast<int>(x);
    const int y0 = static_cast<int>(y);
    const int x1 = x0 + 1;
    const int y1 = y0 + 1;

    // interpolation weights
    const float sx = x - (float)x0;
    const float sy = y - (float)y0;

    // top two corners
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    const float ix0 = cubicInterp(n0, n1, sx);

    // bottom two corners
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    const float ix1 = cubicInterp(n0, n1, sx);

    // combine top and bottom
    return cubicInterp(ix0, ix1, sy);
}

} // namespace Noise
