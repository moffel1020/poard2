#include "terrain_gen.h"

std::vector<Vertex> Terrain::genHeightmapHost() {
    constexpr uint32_t w = chunkSize;
    constexpr uint32_t h = chunkSize;

    constexpr uint32_t octaves = 12;
    constexpr float gridSize = 200;
    constexpr float lacunarity = 2;
    constexpr float gain = 0.5;

    std::vector<Vertex> heightmap;
    heightmap.reserve(w * h);

    for (uint32_t z = 0; z < h; z++) {
        for (uint32_t x = 0; x < w; x++) {
            float val = 0;
            float freq = 1;
            float amp = 1;

            for (uint32_t i = 0; i < octaves; i++) {
                val += Noise::perlin(x * freq / gridSize, z * freq / gridSize) * amp;
                freq *= lacunarity;
                amp *= gain;
            }

            val = glm::clamp(val * 1.2f, -1.0f, 1.0f);

            float height = (((val + 1.0f) * 0.5f));
            heightmap.emplace_back(Vertex{{x, height, z}, {0.0f, 0.0f}});
        }
    }

    return heightmap;
}

std::vector<uint32_t> Terrain::genHeightIndicesHost() {
    constexpr uint32_t w = chunkSize;
    constexpr uint32_t h = chunkSize;

    std::vector<uint32_t> indices;
    indices.reserve((w - 1) * (h - 1) * 2 * 3);

    for (uint32_t j = 0; j < h - 1; j++) {
        for (uint32_t i = 0; i < w - 1; i++) {
            int32_t tl = i + j * w;
            int32_t tr = i + 1 + j * w;
            int32_t bl = i + (j + 1) * w;
            int32_t br = i + 1 + (j + 1) * w;

            indices.push_back(tl);
            indices.push_back(bl);
            indices.push_back(tr);

            indices.push_back(bl);
            indices.push_back(br);
            indices.push_back(tr);
        }
    }

    return indices;
}
