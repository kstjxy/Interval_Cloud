/**
 * ComputeInterval.cs.slang
 *
 * Compute shader for computing front and back depth intervals per pixel from a tet mesh.
 *
 * GPU STRUCTURE DEFINITION FOR TRACK 3:
 *
 * struct TetVertex {
 *     float3 position;  // Vertex position in world space
 * };
 *
 * Tet mesh connectivity: 4 indices per tet, referring to TetVertex positions.
 */

// Structured buffers for tet mesh data
StructuredBuffer<float3> gTetVertices;  // TetVertex.position stored as float3
StructuredBuffer<uint> gTetIndices;     // 4 indices per tet

// Metadata
cbuffer PerFrameCB {
    uint gTetCount;  // Number of tetrahedra
};

// Output texture: per-pixel (front, back) intervals
RWTexture2D<float2> gIntervalOut;

/**
 * Compute front and back depth intervals for a single pixel.
 *
 * For now: placeholder implementation that stores simple debug values.
 * Track 3 will replace this with actual ray-tet intersection and interval computation.
 *
 * Returns: float2(front_depth, back_depth)
 *          If no intersection: (1.0, 0.0) or similar sentinel
 */
float2 computePixelInterval(uint2 pixelCoord, uint2 resolution) {
    // Placeholder: compute normalized pixel coordinates
    float2 uv = float2(pixelCoord) / float2(resolution);

    // Debug: return gradient based on pixel position
    // This helps verify the shader is running and writing correct output
    float frontDepth = uv.x;  // 0 -> 1 left to right
    float backDepth = uv.y;   // 0 -> 1 top to bottom

    return float2(frontDepth, backDepth);
}

[numthreads(16, 16, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    uint2 pixel = dispatchThreadId.xy;

    // Get output dimensions
    uint2 resolution;
    gIntervalOut.GetDimensions(resolution.x, resolution.y);

    // Bounds check
    if (any(pixel >= resolution)) {
        return;
    }

    // Compute interval for this pixel
    float2 interval = computePixelInterval(pixel, resolution);

    // Write to output texture
    gIntervalOut[pixel] = interval;
}
