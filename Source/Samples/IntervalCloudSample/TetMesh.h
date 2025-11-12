#pragma once
#include "Falcor.h"
#include <vector>
#include <fstream>
#include <stdexcept>
#include "Utils/Logger.h"

using namespace Falcor;

/**
 * TetVertex - GPU-compatible vertex structure for tetrahedron mesh.
 *
 * Layout (must match SLANG shader definition):
 * - float3 position: vertex position in world space
 */
struct TetVertex
{
    float3 position;
};

/**
 * TetMesh - CPU-side tetrahedron mesh representation.
 *
 * Stores vertex positions and tetrahedron connectivity.
 * Vertices are indexed via 4 indices per tetrahedron (uint32_t per vertex in tet).
 */
class TetMesh
{
public:
    TetMesh() = default;
    ~TetMesh() = default;

    // Vertex data
    std::vector<TetVertex> vertices;

    // Tet indices: 4 indices per tet, referring to vertex positions
    // Layout: [v0_tet0, v1_tet0, v2_tet0, v3_tet0, v0_tet1, ...]
    std::vector<uint32_t> tetIndices;

    // Metadata
    uint32_t getTetCount() const { return (uint32_t)tetIndices.size() / 4; }
    uint32_t getVertexCount() const { return (uint32_t)vertices.size(); }

    /**
     * Create a simple single-tet mesh for testing.
     *
     * Vertices form a regular tetrahedron:
     * - v0: (0, 1, 0)  - top
     * - v1: (1, -1, 1) - front-right
     * - v2: (-1, -1, 1) - back-right
     * - v3: (0, -1, -1) - back
     */
    static TetMesh createSingleTet()
    {
        TetMesh mesh;

        // Add 4 vertices of a regular-ish tetrahedron
        mesh.vertices.push_back({{0.0f, 1.0f, 0.0f}});     // v0: top
        mesh.vertices.push_back({{1.0f, -1.0f, 1.0f}});    // v1: front-right
        mesh.vertices.push_back({{-1.0f, -1.0f, 1.0f}});   // v2: back-right
        mesh.vertices.push_back({{0.0f, -1.0f, -1.0f}});   // v3: back

        // Single tet with all 4 vertices
        mesh.tetIndices.push_back(0);
        mesh.tetIndices.push_back(1);
        mesh.tetIndices.push_back(2);
        mesh.tetIndices.push_back(3);

        return mesh;
    }

    /**
     * Load tet mesh from a simple text file format.
     *
     * Format:
     *   <num_vertices>
     *   x y z  (vertex 0)
     *   x y z  (vertex 1)
     *   ...
     *   <num_tets>
     *   i0 i1 i2 i3  (tet 0: 4 vertex indices)
     *   i0 i1 i2 i3  (tet 1)
     *   ...
     *
     * Returns empty mesh if file not found or parsing fails.
     */
    static TetMesh loadFromFile(const std::string& filePath)
    {
        TetMesh mesh;

        // Try to open file
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            logWarning("Failed to open tet mesh file: " + filePath);
            return mesh;
        }

        try
        {
            // Read vertices
            uint32_t vertexCount = 0;
            file >> vertexCount;
            if (vertexCount == 0 || vertexCount > 1000000)
                throw std::runtime_error("Invalid vertex count");

            mesh.vertices.resize(vertexCount);
            for (uint32_t i = 0; i < vertexCount; ++i)
            {
                float x, y, z;
                if (!(file >> x >> y >> z))
                    throw std::runtime_error("Failed to read vertex data");
                mesh.vertices[i].position = {x, y, z};
            }

            // Read tets
            uint32_t tetCount = 0;
            file >> tetCount;
            if (tetCount == 0 || tetCount > 1000000)
                throw std::runtime_error("Invalid tet count");

            mesh.tetIndices.reserve(tetCount * 4);
            for (uint32_t i = 0; i < tetCount; ++i)
            {
                uint32_t i0, i1, i2, i3;
                if (!(file >> i0 >> i1 >> i2 >> i3))
                    throw std::runtime_error("Failed to read tet indices");

                // Validate indices
                if (i0 >= vertexCount || i1 >= vertexCount ||
                    i2 >= vertexCount || i3 >= vertexCount)
                    throw std::runtime_error("Index out of range");

                mesh.tetIndices.push_back(i0);
                mesh.tetIndices.push_back(i1);
                mesh.tetIndices.push_back(i2);
                mesh.tetIndices.push_back(i3);
            }

            logInfo("Loaded tet mesh from " + filePath +
                    ": " + std::to_string(vertexCount) + " vertices, " +
                    std::to_string(tetCount) + " tets");
        }
        catch (const std::exception& e)
        {
            logError("Error loading tet mesh: " + std::string(e.what()));
            mesh.vertices.clear();
            mesh.tetIndices.clear();
        }

        return mesh;
    }
};
