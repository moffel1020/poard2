#pragma once
#include "shader_program.h"
#include <glm/glm.hpp>
#include <noise.h>
#include <vector>

// sync with terrain.comp
struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;
};

namespace Terrain {

constexpr uint32_t chunkSize = 4096;                                    // width and height of the chunk
constexpr size_t elemCount = (chunkSize - 1) * (chunkSize - 1) * 2 * 3; // index buffer count

static_assert(chunkSize % 8 == 0, "chunk size must be divisible by 8. keep in sync with layout in compute shader");

// generate heightmap on the cpu
std::vector<Vertex> genHeightmapHost();
std::vector<uint32_t> genHeightIndicesHost();

// generate heightmap on the gpu
inline void genHeightmapDevice(const ShaderProgram& terrainShader, uint32_t vertexId) {
    terrainShader.bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexId);
    glDispatchCompute(chunkSize / 8, chunkSize / 8, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

constexpr size_t getVertexBufferSize() { return chunkSize * chunkSize * sizeof(Vertex); }
constexpr size_t getIndexBufferSize() { return elemCount * sizeof(uint32_t); }

} // namespace Terrain
