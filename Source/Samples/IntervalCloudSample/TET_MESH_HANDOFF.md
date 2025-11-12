# Tet Mesh Data Model & GPU Interface

**Track 2 Completion Handoff to Track 3**

## Overview

Track 2 has implemented the CPU-side tet mesh data structure and GPU upload pipeline. This document specifies the exact GPU interface that Track 3 mesh shaders must implement.

---

## 1. CPU-Side TetMesh Structure (C++)

**File:** `TetMesh.h`

### TetVertex Struct (GPU-compatible layout)

```cpp
struct TetVertex {
    float3 position;  // Vertex position in world/model space
};
```

**Size in memory:** 12 bytes (3 × float32)

### TetMesh Class

```cpp
class TetMesh {
    std::vector<TetVertex> vertices;  // All mesh vertices
    std::vector<uint32_t> tetIndices; // Connectivity: 4 indices per tet

    uint32_t getTetCount() const;     // Number of tets (tetIndices.size() / 4)
    uint32_t getVertexCount() const;  // Number of vertices

    static TetMesh createSingleTet(); // Factory: hardcoded test tet
};
```

**Tet Index Layout:**
- Sequential packing: 4 consecutive uint32 values per tetrahedron
- Each index refers to a vertex in the `vertices` list (0-based)
- **Example:** For 2 tets:
  ```
  tetIndices = [v0_tet0, v1_tet0, v2_tet0, v3_tet0,
                v0_tet1, v1_tet1, v2_tet1, v3_tet1]
  ```

---

## 2. GPU Buffers (SLANG Compute Shader Interface)

**File:** `ComputeInterval.cs.slang`

### Structured Buffers

```slang
StructuredBuffer<float3> gTetVertices;  // TetVertex positions (no padding)
StructuredBuffer<uint> gTetIndices;     // 4 indices per tet, packed sequentially
```

### Constant Buffer

```slang
cbuffer PerFrameCB {
    uint gTetCount;  // Number of tetrahedra
};
```

### Output Texture

```slang
RWTexture2D<float2> gIntervalOut;  // Format: RG16Float
                                    // R = front depth
                                    // G = back depth
```

---

## 3. Shader Integration Pattern

Track 3 should replace the `computePixelInterval()` function in `ComputeInterval.cs.slang` with the actual ray-tet intersection logic.

### Function Signature

```slang
float2 computePixelInterval(uint2 pixelCoord, uint2 resolution) {
    // TRACK 3: Implement ray-tet mesh intersection here

    // Input: pixel coordinates and resolution
    // Output: float2(frontDepth, backDepth)
    //         - frontDepth: closest intersection distance (or 1.0 if none)
    //         - backDepth: farthest intersection distance (or 0.0 if none)

    // Access tet data:
    // - gTetVertices[i] → float3 position
    // - gTetIndices[tetIdx * 4 + localIdx] → vertex index for tet
    // - gTetCount → total number of tets
}
```

### Example Ray-Tet Access Pattern

```slang
// For a given tet index (tetIdx from 0 to gTetCount-1):
uint baseIdx = tetIdx * 4;
uint idx0 = gTetIndices[baseIdx + 0];
uint idx1 = gTetIndices[baseIdx + 1];
uint idx2 = gTetIndices[baseIdx + 2];
uint idx3 = gTetIndices[baseIdx + 3];

float3 v0 = gTetVertices[idx0];
float3 v1 = gTetVertices[idx1];
float3 v2 = gTetVertices[idx2];
float3 v3 = gTetVertices[idx3];

// Now compute ray-tet intersection with (v0, v1, v2, v3)
```

---

## 4. Hardcoded Test Tet

For day 1 testing, `TetMesh::createSingleTet()` provides a simple tetrahedron:

**Vertices (in world space):**
- v0: (0.0,  1.0,  0.0)   - top
- v1: (1.0, -1.0,  1.0)   - front-right
- v2: (-1.0, -1.0,  1.0)  - back-right
- v3: (0.0, -1.0, -1.0)   - back

**Connectivity:**
- Single tet indices: [0, 1, 2, 3]

---

## 5. Current Placeholder

The shader currently outputs a debug gradient (based on pixel position) to allow visual verification that the compute pipeline is working. Track 3 should replace this with real ray-tet computation while maintaining:

- Same buffer layout
- Same output texture format (RG16Float)
- Same function signature for `computePixelInterval()`

---

## 6. Future Extensions (Post Day 1)

### Mesh Importer
- Optional Track 2 task: load .txt or .obj files
- Format: ASCII positions + tet indices
- Call `TetMesh::loadFromFile()` instead of `createSingleTet()`

### Per-Mesh Transforms
- May add world-to-mesh transform matrix to constant buffer
- Allows multiple tet meshes or instancing

### Performance Optimization
- GPU prefix sum for tetIndices (avoid index array)
- Compute shader thread group optimization for large meshes

---

## 7. Files in This Handoff

| File | Purpose |
|------|---------|
| `TetMesh.h` | CPU data structures and factory |
| `IntervalPass.h/cpp` | GPU buffer management and compute dispatch |
| `ComputeInterval.cs.slang` | SLANG compute shader with tet access interface |
| `TET_MESH_HANDOFF.md` | This document (Track 3 reference) |

---

## Questions for Track 3

1. **Ray origin/direction:** How should the camera ray be computed from pixel coordinates?
2. **Interval bounds:** Should unmapped depths be clamped to [0, 1] or use different sentinels?
3. **Multi-tet handling:** Should we compute union or intersection of intervals across tets?

---

**Status:** ✓ GPU buffers loaded and bound
**Next:** Track 3 implements ray-tet intersection in `computePixelInterval()`
