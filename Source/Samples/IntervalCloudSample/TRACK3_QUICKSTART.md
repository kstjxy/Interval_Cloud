# Track 3 Quick Start: Implementing Mesh Shader

## Your Job

Replace the placeholder in `ComputeInterval.cs.slang` with real ray-tet intersection code.

---

## The Function You Need to Implement

**File:** `Source/Samples/IntervalCloudSample/passes/ComputeInterval.cs.slang`

**Find this:**
```slang
float2 computePixelInterval(uint2 pixelCoord, uint2 resolution) {
    // Placeholder: compute normalized pixel coordinates
    float2 uv = float2(pixelCoord) / float2(resolution);
    float frontDepth = uv.x;
    float backDepth = uv.y;
    return float2(frontDepth, backDepth);
}
```

**Replace with your implementation:**
```slang
float2 computePixelInterval(uint2 pixelCoord, uint2 resolution) {
    // TRACK 3 TODO:
    // 1. Generate ray from camera through pixel
    // 2. Iterate through all tets and find intersections
    // 3. Return closest (front) and farthest (back) intersection depths
    // 4. Return (1.0, 0.0) or sentinel if no intersection
}
```

---

## Available GPU Resources

You have access to:

```slang
// Tet mesh data
StructuredBuffer<float3> gTetVertices;  // All vertex positions
StructuredBuffer<uint> gTetIndices;     // 4 indices per tet, packed sequentially

// Metadata
cbuffer PerFrameCB {
    uint gTetCount;  // Number of tets to test
};

// Output texture (already allocated)
RWTexture2D<float2> gIntervalOut;  // You write here: R=frontDepth, G=backDepth
```

---

## How to Access Tet Data

### Loop through all tets:
```slang
for (uint tetIdx = 0; tetIdx < gTetCount; ++tetIdx) {
    // Get tet vertices...
}
```

### Get vertices of tet `tetIdx`:
```slang
uint baseIdx = tetIdx * 4;
uint idx0 = gTetIndices[baseIdx + 0];
uint idx1 = gTetIndices[baseIdx + 1];
uint idx2 = gTetIndices[baseIdx + 2];
uint idx3 = gTetIndices[baseIdx + 3];

float3 v0 = gTetVertices[idx0];
float3 v1 = gTetVertices[idx1];
float3 v2 = gTetVertices[idx2];
float3 v3 = gTetVertices[idx3];
```

---

## Algorithm Outline

```slang
float2 computePixelInterval(uint2 pixelCoord, uint2 resolution) {
    // Step 1: Generate camera ray
    float3 rayOrigin = /* camera position */;
    float3 rayDir = /* camera ray direction for this pixel */;

    // Step 2: Initialize interval
    float frontDepth = 1.0;    // Closest intersection (or 1.0 if none)
    float backDepth = 0.0;     // Farthest intersection (or 0.0 if none)

    // Step 3: Test ray against all tets
    for (uint tetIdx = 0; tetIdx < gTetCount; ++tetIdx) {
        float4 v0 = float4(gTetVertices[gTetIndices[tetIdx * 4 + 0]], 1.0);
        float4 v1 = float4(gTetVertices[gTetIndices[tetIdx * 4 + 1]], 1.0);
        float4 v2 = float4(gTetVertices[gTetIndices[tetIdx * 4 + 2]], 1.0);
        float4 v3 = float4(gTetVertices[gTetIndices[tetIdx * 4 + 3]], 1.0);

        // Ray-tet intersection (your algorithm here)
        float entryDepth, exitDepth;
        if (rayTetIntersect(rayOrigin, rayDir, v0.xyz, v1.xyz, v2.xyz, v3.xyz,
                            entryDepth, exitDepth)) {
            // Update front/back
            frontDepth = min(frontDepth, entryDepth);
            backDepth = max(backDepth, exitDepth);
        }
    }

    return float2(frontDepth, backDepth);
}
```

---

## Debugging Tips

### 1. Verify shader loads
The gradient output should work. If not, there's a build/compile error.

### 2. Output intermediate values
Replace return with debug visualization:
```slang
// Debug: visualize whether we found intersections
if (frontDepth < 1.0) {
    return float2(1.0, 0.0);  // Red: hit
} else {
    return float2(0.0, 1.0);  // Green: miss
}
```

### 3. Verify tet data
```slang
// Debug: draw vertices as colors
if (gTetCount > 0) {
    float3 v0 = gTetVertices[gTetIndices[0]];
    return float2(abs(v0.x), abs(v0.y));  // Visualize v0 coordinates
}
```

### 4. Test single tet first
The hardcoded single tet is at positions:
- v0: (0, 1, 0)
- v1: (1, -1, 1)
- v2: (-1, -1, 1)
- v3: (0, -1, -1)

Your ray-tet code should intersect this visible shape.

---

## Reference: Ray-Tet Intersection

You'll need to implement or find a ray-tetrahedron intersection algorithm.

### Common approaches:
1. **Barycentric coordinates** - Express tet as weighted sum of vertices
2. **Plane-plane-plane intersection** - Find ray intersections with 4 faces
3. **AABB bounding** - Quick reject, then detailed intersection

### Pseudo-code (Möller-Trumbore variant for tet):
```slang
bool rayTetIntersect(float3 rayOrigin, float3 rayDir,
                     float3 v0, float3 v1, float3 v2, float3 v3,
                     out float entryDepth, out float exitDepth) {
    // For each of 4 tet faces:
    //   1. Compute ray-face intersection
    //   2. Check if inside tet barycentric space
    // Return closest entry and farthest exit
}
```

Check NVIDIA's research papers on ray-tet intersection (used in volumetric ray tracing).

---

## Performance Considerations

- **Loop unrolling:** Consider unrolling the 4-tet-face loop
- **Early exit:** Stop if backDepth already found and frontDepth < new_entry
- **Register pressure:** Be careful with temp variables
- **Branching:** Minimize if/else (GPU prefers data parallelism)

---

## When You're Done

1. **Build & run** - Should compile without errors
2. **Visual check** - Does output show intersections with test tet?
3. **Test with DebugViewPass** - Toggle Front/Back/Length views
4. **Profile** - Check GPU utilization with NSight

---

## Questions?

Refer to `TET_MESH_HANDOFF.md` for complete data structure documentation.

---

Good luck! 🚀
