# Track 2 Implementation Guide: Tet Data Model & Loader

**Purpose:** Complete reference for understanding and extending the tet mesh infrastructure.

**For:** Teammates working on mesh loading, GPU integration, or tet-based rendering.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Core Interfaces & APIs](#core-interfaces--apis)
3. [CPU-Side Data Structures](#cpu-side-data-structures)
4. [GPU Buffer Management](#gpu-buffer-management)
5. [Integration Points](#integration-points)
6. [Implementation Details](#implementation-details)
7. [How to Extend](#how-to-extend)
8. [Common Patterns](#common-patterns)
9. [Troubleshooting](#troubleshooting)

---

## Architecture Overview

### System Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                  APPLICATION LAYER                          │
│                                                              │
│  IntervalCloudSample::onFrameRender()                      │
│    ↓                                                         │
│  RenderGraph executes passes in sequence                    │
│    ↓                                                         │
│  ClearPass → IntervalPass → DebugViewPass → Screen         │
└──────────────────────┬──────────────────────────────────────┘
                       ↓
┌─────────────────────────────────────────────────────────────┐
│                 INTERVAL PASS (TRACK 2)                     │
│                                                              │
│  Constructor: Initialize state, wait for RenderContext     │
│    ↓                                                         │
│  First execute(): Call loadTetMesh()                        │
│    ↓                                                         │
│  loadTetMesh():                                             │
│    1. Create CPU TetMesh (hardcoded or loaded)             │
│    2. Upload vertices → GPU structured buffer              │
│    3. Upload indices → GPU structured buffer               │
│    4. Prepare for Track 3 compute shader                   │
│    ↓                                                         │
│  Every execute(): Output interval texture (RG16Float)      │
└──────────────────────┬──────────────────────────────────────┘
                       ↓
┌─────────────────────────────────────────────────────────────┐
│               GPU SIDE (READY FOR TRACK 3)                  │
│                                                              │
│  ComputeInterval.cs.slang:                                 │
│    - Read: gTetVertices[], gTetIndices[]                   │
│    - Compute: ray-tet intersection per pixel               │
│    - Write: gIntervalOut[] (front, back depths)            │
└─────────────────────────────────────────────────────────────┘
```

---

## Core Interfaces & APIs

### 1. TetMesh Class (CPU Side)

**File:** `Source/Samples/IntervalCloudSample/TetMesh.h`

#### Data Members

```cpp
class TetMesh {
public:
    // Vertex data - all vertex positions
    std::vector<TetVertex> vertices;

    // Index data - 4 indices per tet, packed sequentially
    std::vector<uint32_t> tetIndices;
    // Example: for 2 tets:
    // [v0_tet0, v1_tet0, v2_tet0, v3_tet0, v0_tet1, v1_tet1, v2_tet1, v3_tet1]
};
```

#### Public Methods

```cpp
// Metadata queries
uint32_t getTetCount() const;      // Returns tetIndices.size() / 4
uint32_t getVertexCount() const;   // Returns vertices.size()

// Factory - hardcoded test pyramid
static TetMesh createSingleTet();

// File I/O - load from text format
static TetMesh loadFromFile(const std::string& filePath);
```

#### Text File Format for loadFromFile()

```
<num_vertices>
x0 y0 z0        # vertex 0 position
x1 y1 z1        # vertex 1 position
...
<num_tets>
i0 i1 i2 i3     # tet 0: 4 vertex indices
i0 i1 i2 i3     # tet 1
...
```

**Example:**
```
4
0 1 0
1 -1 1
-1 -1 1
0 -1 -1
1
0 1 2 3
```

### 2. TetVertex Structure

**File:** `Source/Samples/IntervalCloudSample/TetMesh.h`

```cpp
struct TetVertex {
    float3 position;  // 12 bytes, GPU-aligned
};
```

**Memory Layout:**
```
Offset  Size  Type    Field
0       4     float   position.x
4       4     float   position.y
8       4     float   position.z
```

**Total:** 12 bytes per vertex (no padding)

### 3. IntervalPass Class (GPU Integration)

**File:** `Source/Samples/IntervalCloudSample/passes/IntervalPass.h`

#### Private Members

```cpp
class IntervalPass : public RenderPass {
private:
    // CPU-side mesh data
    TetMesh mTetMesh;

    // GPU buffers
    ref<Buffer> mpTetVertexBuffer;   // Structured buffer: TetVertex[]
    ref<Buffer> mpTetIndexBuffer;    // Structured buffer: uint[]

    // Compute pass (for Track 3)
    ref<ComputePass> mpComputePass;
    ref<ProgramVars> mpVars;

    // State tracking
    bool mbMeshLoaded = false;

    // Initialization
    void loadTetMesh(RenderContext* pRenderContext);
};
```

#### Public Methods

```cpp
// RenderPass interface (required)
static ref<IntervalPass> create(ref<Device> pDevice, const Properties& props);
RenderPassReflection reflect(const CompileData& compileData) override;
void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
```

---

## CPU-Side Data Structures

### TetMesh Class Detailed Reference

#### Creating a Mesh

```cpp
// Option 1: Hardcoded test pyramid
TetMesh mesh = TetMesh::createSingleTet();
// Result: 4 vertices forming a pyramid, 1 tet

// Option 2: Load from file
TetMesh mesh = TetMesh::loadFromFile("data/my_mesh.txt");
// Result: Custom mesh from text file

// Option 3: Programmatically construct
TetMesh mesh;
mesh.vertices.push_back({float3(0, 0, 0)});
mesh.vertices.push_back({float3(1, 0, 0)});
mesh.vertices.push_back({float3(0, 1, 0)});
mesh.vertices.push_back({float3(0, 0, 1)});
mesh.tetIndices = {0, 1, 2, 3};  // Single tet
```

#### Accessing Tet Data

```cpp
// Get all vertices
uint32_t vertexCount = mesh.getVertexCount();
for (uint32_t i = 0; i < vertexCount; ++i) {
    float3 pos = mesh.vertices[i].position;
    // Use position...
}

// Get all tets
uint32_t tetCount = mesh.getTetCount();
for (uint32_t i = 0; i < tetCount; ++i) {
    // Get 4 vertex indices for tet i
    uint32_t idx0 = mesh.tetIndices[i * 4 + 0];
    uint32_t idx1 = mesh.tetIndices[i * 4 + 1];
    uint32_t idx2 = mesh.tetIndices[i * 4 + 2];
    uint32_t idx3 = mesh.tetIndices[i * 4 + 3];

    // Get vertices
    float3 v0 = mesh.vertices[idx0].position;
    float3 v1 = mesh.vertices[idx1].position;
    float3 v2 = mesh.vertices[idx2].position;
    float3 v3 = mesh.vertices[idx3].position;
}
```

---

## GPU Buffer Management

### Creating Structured Buffers

**API:** Falcor's Device class

```cpp
// Vertex buffer - read-only in shader
ref<Buffer> vertexBuffer = pDevice->createStructuredBuffer(
    elementCount,        // uint32_t: number of TetVertex elements
    elementSize,         // uint32_t: sizeof(TetVertex) = 12
    ResourceBindFlags::ShaderResource,  // Bind as SRV (read)
    MemoryType::DeviceLocal,            // GPU memory
    data                 // const void*: pointer to CPU data
);

// Index buffer - read-only in shader
ref<Buffer> indexBuffer = pDevice->createStructuredBuffer(
    elementCount,        // uint32_t: number of uint32_t elements (4*numTets)
    sizeof(uint32_t),    // uint32_t: 4 bytes
    ResourceBindFlags::ShaderResource,
    MemoryType::DeviceLocal,
    data
);
```

### Binding to Shaders

**Pattern 1: Via ProgramVars (preferred)**

```cpp
ref<ComputePass> computePass = ComputePass::create(pDevice, shaderPath);
ref<ProgramVars> vars = computePass->getVars();

auto rootVar = vars->getRootVar();
rootVar["gTetVertices"] = vertexBuffer;     // Bind vertex buffer
rootVar["gTetIndices"] = indexBuffer;       // Bind index buffer
rootVar["gTetCount"] = meshData.getTetCount();  // Scalar constant
```

**Pattern 2: Via ParameterBlock (advanced)**

```cpp
ref<ParameterBlock> paramBlock = ParameterBlock::create(
    pDevice,
    pProgram,
    "TetMeshData"
);
paramBlock["vertices"] = vertexBuffer;
paramBlock["indices"] = indexBuffer;
vars->setParameterBlock("gTetMesh", paramBlock);
```

### GPU Memory Layout

```
Vertex Buffer (Device Local)
┌──────────────────────────────┐
│ TetVertex 0: (x0, y0, z0)   │  12 bytes
├──────────────────────────────┤
│ TetVertex 1: (x1, y1, z1)   │  12 bytes
├──────────────────────────────┤
│ TetVertex 2: (x2, y2, z2)   │  12 bytes
├──────────────────────────────┤
│ TetVertex 3: (x3, y3, z3)   │  12 bytes
└──────────────────────────────┘
Total: 48 bytes for 4 vertices

Index Buffer (Device Local)
┌──────────────────────────────┐
│ uint 0 (tet 0, v0)          │  4 bytes
├──────────────────────────────┤
│ uint 1 (tet 0, v1)          │  4 bytes
├──────────────────────────────┤
│ uint 2 (tet 0, v2)          │  4 bytes
├──────────────────────────────┤
│ uint 3 (tet 0, v3)          │  4 bytes
└──────────────────────────────┘
Total: 16 bytes for 1 tet (4 indices)
```

---

## Integration Points

### 1. RenderPass Reflection

```cpp
RenderPassReflection IntervalPass::reflect(const CompileData& compileData) {
    RenderPassReflection reflector;

    // Input: Color texture from ClearPass
    reflector.addInput(kColorIn, "Scene color buffer")
        .format(ResourceFormat::RGBA8UnormSrgb)
        .texture2D()
        .bindFlags(ResourceBindFlags::ShaderResource);

    // Output: Interval texture (front/back depths)
    reflector.addOutput(kIntervalOut, "Interval texture")
        .format(ResourceFormat::RG16Float)
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);

    return reflector;
}
```

### 2. Lazy Initialization Pattern

```cpp
void IntervalPass::execute(RenderContext* pRenderContext, const RenderData& renderData) {
    // ... validation ...

    // Load mesh only on first frame
    if (!mbMeshLoaded) {
        loadTetMesh(pRenderContext);  // RenderContext available here
    }

    // Process every frame
    // ...
}
```

### 3. Resource Lifecycle

```
Constructor → First Execute → Every Execute → Destruction
   |              |               |              |
(Create state)   Load GPU       Use GPU      Auto cleanup
   |         data on GPU       resources    via ref<>
```

---

## Implementation Details

### loadTetMesh() Function

**Location:** `IntervalPass.cpp`

**Responsibility:** One-time initialization of GPU resources

```cpp
void IntervalPass::loadTetMesh(RenderContext* pRenderContext) {
    auto pDevice = mpDevice;  // Get device from RenderPass base class

    // Step 1: Create CPU mesh
    mTetMesh = TetMesh::createSingleTet();
    // Alternative: TetMesh::loadFromFile("path/to/mesh.txt")

    // Step 2: Upload vertices to GPU
    uint32_t vertexCount = mTetMesh.getVertexCount();
    uint32_t vertexSize = sizeof(TetVertex);
    mpTetVertexBuffer = pDevice->createStructuredBuffer(
        vertexCount,
        vertexSize,
        ResourceBindFlags::ShaderResource,  // Read-only SRV
        MemoryType::DeviceLocal,            // GPU memory
        mTetMesh.vertices.data()            // CPU data → GPU
    );

    // Step 3: Upload indices to GPU
    uint32_t indexCount = (uint32_t)mTetMesh.tetIndices.size();
    mpTetIndexBuffer = pDevice->createStructuredBuffer(
        indexCount,
        sizeof(uint32_t),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mTetMesh.tetIndices.data()
    );

    // Step 4: Validation logging (debug)
    logInfo("Loaded " + std::to_string(mTetMesh.getTetCount()) +
            " tets, " + std::to_string(vertexCount) + " vertices");

    mbMeshLoaded = true;  // Mark as initialized
}
```

### execute() Function

**Location:** `IntervalPass.cpp`

**Responsibility:** Per-frame rendering

```cpp
void IntervalPass::execute(RenderContext* pRenderContext, const RenderData& renderData) {
    // Get input/output textures
    FALCOR_ASSERT(renderData[kColorIn] != nullptr, "Missing color input");
    ref<Texture> pIntervalOut = renderData.getTexture(kIntervalOut);
    if (!pIntervalOut) return;

    // Initialize GPU resources once
    if (!mbMeshLoaded) {
        loadTetMesh(pRenderContext);
    }

    // For now: output placeholder (Track 2)
    // Track 3 will replace with compute shader execution
    float4 debugColor(0.2f, 0.5f, 0.8f, 1.0f);  // Blue
    pRenderContext->clearTexture(pIntervalOut.get(), debugColor);
}
```

---

## How to Extend

### Adding Custom Attributes to TetVertex

**Current:**
```cpp
struct TetVertex {
    float3 position;
};
```

**Extended (example):**
```cpp
struct TetVertex {
    float3 position;     // 12 bytes
    float3 normal;       // 12 bytes (optional)
    uint32_t materialId; // 4 bytes (optional)
};  // Total: 28 bytes per vertex
```

**After change:**
1. Update `sizeof(TetVertex)` in buffer creation
2. Update GPU shader struct definition
3. Update TetMesh file format if needed

### Supporting Multiple Meshes

**Current:** Single mesh in IntervalPass

**Extension:**
```cpp
class IntervalPass {
private:
    struct MeshData {
        TetMesh cpu_mesh;
        ref<Buffer> vertex_buffer;
        ref<Buffer> index_buffer;
        uint32_t vertex_offset;
        uint32_t index_offset;
    };

    std::vector<MeshData> mMeshes;  // Multiple meshes
    ref<Buffer> mpCombinedVertexBuffer;  // All vertices
    ref<Buffer> mpCombinedIndexBuffer;   // All indices
};
```

### Adding Per-Mesh Transforms

**Extension:**
```cpp
struct MeshTransform {
    float4x4 worldToLocal;
    float4x4 localToWorld;
};

// In constant buffer
cbuffer PerFrameCB {
    uint gTetCount;
    uint gMeshCount;
    MeshTransform gMeshTransforms[MAX_MESHES];
};
```

### Loading Different Mesh Formats

**Current:** Text file format via `loadFromFile()`

**Extensions:**
```cpp
// .obj importer
static TetMesh loadFromOBJ(const std::string& filePath);

// .vtu (VTK Unstructured Grid)
static TetMesh loadFromVTU(const std::string& filePath);

// Procedural generation
static TetMesh generateGrid(uint32_t gridSize, float spacing);
static TetMesh generateSphere(uint32_t subdivisions);
```

---

## Common Patterns

### Pattern 1: Creating a Buffer

```cpp
// Step 1: Prepare CPU data
std::vector<TetVertex> vertices = {...};
uint32_t vertexCount = vertices.size();

// Step 2: Create GPU buffer
ref<Buffer> gpuBuffer = pDevice->createStructuredBuffer(
    vertexCount,
    sizeof(TetVertex),
    ResourceBindFlags::ShaderResource,
    MemoryType::DeviceLocal,
    vertices.data()
);

// Step 3: Use buffer (bind to shader, etc.)
```

### Pattern 2: Accessing Tet Connectivity

```cpp
uint32_t tetIdx = 0;  // Which tet?

// Get the 4 vertex indices for this tet
uint32_t i0 = tetIndices[tetIdx * 4 + 0];
uint32_t i1 = tetIndices[tetIdx * 4 + 1];
uint32_t i2 = tetIndices[tetIdx * 4 + 2];
uint32_t i3 = tetIndices[tetIdx * 4 + 3];

// Get vertex data
TetVertex v0 = vertices[i0];
TetVertex v1 = vertices[i1];
TetVertex v2 = vertices[i2];
TetVertex v3 = vertices[i3];
```

### Pattern 3: Validating Mesh Integrity

```cpp
bool isValidMesh(const TetMesh& mesh) {
    // Check vertex count
    if (mesh.getVertexCount() == 0) return false;

    // Check tet count
    if (mesh.getTetCount() == 0) return false;

    // Check index validity
    for (uint32_t i = 0; i < mesh.getTetCount(); ++i) {
        for (uint32_t j = 0; j < 4; ++j) {
            uint32_t idx = mesh.tetIndices[i * 4 + j];
            if (idx >= mesh.getVertexCount()) return false;
        }
    }

    return true;
}
```

### Pattern 4: Logging Mesh Statistics

```cpp
void logMeshStats(const TetMesh& mesh) {
    logInfo("Mesh Statistics:");
    logInfo("  Vertices: " + std::to_string(mesh.getVertexCount()));
    logInfo("  Tets: " + std::to_string(mesh.getTetCount()));

    uint64_t vertexMemory = mesh.getVertexCount() * sizeof(TetVertex);
    uint64_t indexMemory = mesh.tetIndices.size() * sizeof(uint32_t);
    uint64_t totalMemory = vertexMemory + indexMemory;

    logInfo("  CPU Memory: " + std::to_string(totalMemory) + " bytes");
    logInfo("    Vertices: " + std::to_string(vertexMemory) + " bytes");
    logInfo("    Indices: " + std::to_string(indexMemory) + " bytes");
}
```

---

## Troubleshooting

### Issue: Buffer Creation Fails

**Symptoms:**
- `mpTetVertexBuffer` is nullptr
- No error message

**Solutions:**
1. Check device is valid: `if (!pDevice) logError("Device null");`
2. Check vertex count > 0: `assert(vertexCount > 0);`
3. Check data pointer not null: `assert(vertices.data() != nullptr);`
4. Try debug mode: add logging before createStructuredBuffer()

**Example debug code:**
```cpp
logInfo("Creating vertex buffer...");
logInfo("  Count: " + std::to_string(vertexCount));
logInfo("  Size: " + std::to_string(vertexSize));
logInfo("  Device: " + (pDevice ? "valid" : "NULL"));
logInfo("  Data: " + (vertices.data() ? "valid" : "NULL"));

mpTetVertexBuffer = pDevice->createStructuredBuffer(...);

if (mpTetVertexBuffer)
    logInfo("✓ Buffer created successfully");
else
    logError("✗ Buffer creation failed");
```

### Issue: Wrong Data in GPU

**Symptoms:**
- Buffer created but wrong values in shader
- Values seem like garbage

**Solutions:**
1. Verify CPU data before upload: print vertices to console
2. Check memory layout: ensure TetVertex is 12 bytes exactly
3. Verify stride calculation: `stride = sizeof(TetVertex)`
4. Check alignment: float3 should start at 4-byte boundary

**Validation code:**
```cpp
assert(sizeof(TetVertex) == 12, "TetVertex must be 12 bytes");
assert(offsetof(TetVertex, position) == 0, "position must be at offset 0");
```

### Issue: Mesh Indices Out of Bounds

**Symptoms:**
- GPU error or undefined behavior
- Shader accesses invalid vertices

**Solutions:**
1. Validate indices before GPU upload:
```cpp
for (auto idx : mesh.tetIndices) {
    assert(idx < mesh.getVertexCount(), "Index out of bounds");
}
```

2. Add bounds checking in shader:
```slang
uint idx = gTetIndices[i];
if (idx >= gVertexCount) return;  // Skip invalid
float3 pos = gTetVertices[idx];
```

### Issue: Memory Leak

**Symptoms:**
- Memory usage grows frame by frame
- Leak report on shutdown

**Solutions:**
1. Use `ref<>` for all GPU resources (automatic cleanup)
2. Don't create buffers every frame: use `if (!mbMeshLoaded)`
3. Profile with Visual Studio Debug > Windows > Memory

**Check:**
```cpp
// WRONG - creates buffer every frame
void execute(...) {
    mpBuffer = pDevice->createStructuredBuffer(...);  // Leak!
}

// RIGHT - create once
void loadTetMesh(...) {
    mpBuffer = pDevice->createStructuredBuffer(...);  // Created once
}
```

---

## Summary: Key Takeaways

✅ **CPU Side:**
- `TetMesh` holds vertices and tet indices
- `TetVertex` is 12 bytes: float3 position only
- Indices packed as 4 per tet sequentially

✅ **GPU Side:**
- Structured buffers: `gTetVertices[]` and `gTetIndices[]`
- ShaderResource binding for read-only access
- Constant buffer: `gTetCount`

✅ **Integration:**
- Lazy load in first `execute()` call
- Use `RenderContext` from `execute()` for GPU operations
- ref<> handles automatic resource cleanup

✅ **Extensibility:**
- Add vertex attributes: update struct size
- Multiple meshes: pack into single buffer
- Different formats: implement new loaders

---

**For Track 3:** See `TET_MESH_HANDOFF.md` for GPU shader integration details.

**Questions?** Check `TRACK2_TESTING_GUIDE.md` for debugging tips.
