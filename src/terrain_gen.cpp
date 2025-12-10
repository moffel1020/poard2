#include "terrain_gen.h"

void TerrainGen::genChunk(
    const ShaderProgram& terrainShader, uint32_t vertexId, glm::ivec2 chunkIdx, uint32_t buffIdx) const {

    terrainShader.bind();
    glUniform1ui(glGetUniformLocation(terrainShader.handle(), "gridSize"), config.gridSize);
    glUniform1ui(glGetUniformLocation(terrainShader.handle(), "octaves"), config.octaves);
    glUniform1f(glGetUniformLocation(terrainShader.handle(), "lacunarity"), config.lacunarity);
    glUniform1f(glGetUniformLocation(terrainShader.handle(), "gain"), config.gain);
    glUniform2i(glGetUniformLocation(terrainShader.handle(), "chunkIdx"), chunkIdx.x, chunkIdx.y);
    glUniform2i(glGetUniformLocation(terrainShader.handle(), "centerIdx"), currentCenter.x, currentCenter.y);
    glUniform1ui(glGetUniformLocation(terrainShader.handle(), "buffIdx"), buffIdx);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertexId);
    glDispatchCompute(chunkSize / 8, chunkSize / 8, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void TerrainGen::update(const ShaderProgram& terrainShader, uint32_t vertexId, glm::ivec2 center) {
    currentCenter = center;
    const auto goodChunks = getChunksInRange(center);

    // find new chunks
    std::vector<glm::ivec2> toAlloc;
    for (auto& c : goodChunks) {
        if (allocatedChunks.find(c) == allocatedChunks.end()) {
            toAlloc.push_back(c);
        }
    }

    // find old chunks
    std::vector<std::pair<glm::ivec2, uint32_t>> toFree;
    for (const auto& chunkIdx : allocatedChunks) {
        if (goodChunks.find(chunkIdx.first) == goodChunks.end()) {
            toFree.push_back(chunkIdx);
        }
    }

    // remove old chunks
    for (const auto& [c, _] : toFree) {
        allocatedChunks.erase(c);
    }

    // generate new chunks
    if (toFree.size() == toAlloc.size()) {
        for (uint32_t i = 0; i < toAlloc.size(); i++) {
            const auto& [_, idx] = toFree[i];
            genChunk(terrainShader, vertexId, toAlloc[i], idx);
            allocatedChunks.insert(std::make_pair(toAlloc[i], idx));
        }
    } else {
        assert(toAlloc.size() == chunkCount);
        for (uint32_t i = 0; i < toAlloc.size(); i++) {
            const auto& chunk = toAlloc[i];
            genChunk(terrainShader, vertexId, chunk, i);
            allocatedChunks.insert(std::make_pair(chunk, i));
        }
    }
}

std::unordered_set<glm::ivec2> TerrainGen::getChunksInRange(glm::ivec2 center) const {
    constexpr int32_t R = chunkDistance;

    std::unordered_set<glm::ivec2> points;
    for (int32_t x = center.x - R; x <= center.x + R; x++) {
        int32_t yRange = R - abs(x - center.x);
        for (int32_t y = center.y - yRange; y <= center.y + yRange; ++y) {
            points.insert(glm::ivec2(x, y));
        }
    }

    return points;
}

std::vector<uint32_t> TerrainGen::genHeightIndices() {
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
