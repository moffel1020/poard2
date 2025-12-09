#pragma once
#include "shader_program.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <noise.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// sync with terrain.comp
struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;
};

class TerrainGen {
public:
    static constexpr uint32_t chunkSize = 1024;                                    // width and height of the chunk
    static constexpr size_t elemCount = (chunkSize - 1) * (chunkSize - 1) * 2 * 3; // index buffer count
    static_assert(chunkSize % 8 == 0, "chunk size must be divisible by 8. keep in sync with layout in compute shader");

    static constexpr uint32_t chunkCount = 41; // with manhattan distance 4
    static constexpr uint32_t chunkDistance = 4;


    static constexpr size_t getVertexBufferSize() { return chunkSize * chunkSize * sizeof(Vertex) * chunkCount; }
    static constexpr size_t getIndexBufferSize() { return elemCount * sizeof(uint32_t); }

    static std::vector<uint32_t> genHeightIndicesHost();

    void update(const ShaderProgram& terrainShader, uint32_t vertexId, glm::ivec2 center);

    void clearChunkCache() {
        allocatedChunks.clear();
    }

private:
    std::unordered_set<glm::ivec2> getChunksInRange(glm::ivec2 center) const;
    void genChunk(const ShaderProgram& terrainShader, uint32_t vertexId, glm::ivec2 chunkIdx, uint32_t buffIdx) const;

    glm::ivec2 currentCenter;
    std::unordered_map<glm::ivec2, uint32_t> allocatedChunks;
};
