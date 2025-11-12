# Track 2 Testing Guide: Verifying Your Implementation

This guide shows you how to test that Track 2 (Tet Data Model & Loader) is working correctly.

---

## Test 1: App Runs Without Crashing ✓

**Verification:**
- Launch the app
- It should start without __debugbreak() errors
- You should see a blue window (debug color from IntervalPass)

**What this proves:**
- CPU-side tet mesh structures are valid
- GPU buffers are created successfully
- RenderGraph pipeline executes without errors

---

## Test 2: Verify Tet Mesh Is Created

**Where to add the test:**
Edit `IntervalPass.cpp` in the `loadTetMesh()` function:

```cpp
void IntervalPass::loadTetMesh(RenderContext* pRenderContext)
{
    auto pDevice = mpDevice;

    // Create hardcoded single-tet mesh
    mTetMesh = TetMesh::createSingleTet();

    // ADD THIS TEST:
    logInfo("=== TET MESH CREATED ===");
    logInfo("Vertex count: " + std::to_string(mTetMesh.getVertexCount()));
    logInfo("Tet count: " + std::to_string(mTetMesh.getTetCount()));

    // Print all vertices
    for (uint32_t i = 0; i < mTetMesh.getVertexCount(); ++i)
    {
        float3 pos = mTetMesh.vertices[i].position;
        logInfo("  Vertex " + std::to_string(i) + ": (" +
                std::to_string(pos.x) + ", " +
                std::to_string(pos.y) + ", " +
                std::to_string(pos.z) + ")");
    }

    // Print all tet indices
    for (uint32_t i = 0; i < mTetMesh.getTetCount(); ++i)
    {
        uint32_t idx0 = mTetMesh.tetIndices[i * 4 + 0];
        uint32_t idx1 = mTetMesh.tetIndices[i * 4 + 1];
        uint32_t idx2 = mTetMesh.tetIndices[i * 4 + 2];
        uint32_t idx3 = mTetMesh.tetIndices[i * 4 + 3];
        logInfo("  Tet " + std::to_string(i) + ": [" +
                std::to_string(idx0) + ", " +
                std::to_string(idx1) + ", " +
                std::to_string(idx2) + ", " +
                std::to_string(idx3) + "]");
    }
```

**Expected output in console:**
```
(Info) === TET MESH CREATED ===
(Info) Vertex count: 4
(Info) Tet count: 1
(Info)   Vertex 0: (0, 1, 0)
(Info)   Vertex 1: (1, -1, 1)
(Info)   Vertex 2: (-1, -1, 1)
(Info)   Vertex 3: (0, -1, -1)
(Info)   Tet 0: [0, 1, 2, 3]
```

**What this proves:**
- TetMesh::createSingleTet() works correctly
- Vertex positions are correct
- Tet connectivity is correct (all 4 vertices used)

---

## Test 3: Verify GPU Buffers Are Created

**What to check:**
GPU buffers should be created without errors. Add logging:

```cpp
void IntervalPass::loadTetMesh(RenderContext* pRenderContext)
{
    // ... after creating buffers ...

    logInfo("=== GPU BUFFERS CREATED ===");

    if (mpTetVertexBuffer)
    {
        logInfo("Vertex buffer size: " + std::to_string(mpTetVertexBuffer->getSize()) + " bytes");
        logInfo("Expected size: " + std::to_string(mTetMesh.getVertexCount() * sizeof(TetVertex)) + " bytes");
    }
    else
    {
        logError("Vertex buffer creation FAILED");
    }

    if (mpTetIndexBuffer)
    {
        logInfo("Index buffer size: " + std::to_string(mpTetIndexBuffer->getSize()) + " bytes");
        logInfo("Expected size: " + std::to_string(mTetMesh.tetIndices.size() * sizeof(uint32_t)) + " bytes");
    }
    else
    {
        logError("Index buffer creation FAILED");
    }
```

**Expected output:**
```
(Info) === GPU BUFFERS CREATED ===
(Info) Vertex buffer size: 48 bytes
(Info) Expected size: 48 bytes
(Info) Index buffer size: 16 bytes
(Info) Expected size: 16 bytes
```

**What this proves:**
- Structured buffers are allocated correctly
- Buffer sizes match expected (4 vertices × 12 bytes = 48 bytes)
- Index buffer correct (4 indices × 4 bytes = 16 bytes)

---

## Test 4: Verify Data Transfer to GPU

**Advanced test:** Use PIX or NSight to inspect GPU memory

1. **Using NSight Graphics (NVIDIA):**
   - Launch NSight Graphics
   - Capture a frame of your app
   - Inspect "Buffers" tab
   - Look for `mpTetVertexBuffer` and `mpTetIndexBuffer`
   - Verify values match CPU-side mesh

2. **Simple validation in code:**
```cpp
// In IntervalPass::execute(), after loadTetMesh():

// Create a temporary CPU buffer to read back GPU data
ref<Buffer> readbackVB = pDevice->createBuffer(
    mpTetVertexBuffer->getSize(),
    ResourceBindFlags::None,
    MemoryType::Upload
);

// Copy GPU → CPU
pRenderContext->copyBufferRegion(readbackVB, mpTetVertexBuffer);

// Map and verify
TetVertex* pData = (TetVertex*)pRenderContext->map(readbackVB);
if (pData)
{
    for (uint32_t i = 0; i < mTetMesh.getVertexCount(); ++i)
    {
        logInfo("GPU Vertex " + std::to_string(i) + ": (" +
                std::to_string(pData[i].position.x) + ", " +
                std::to_string(pData[i].position.y) + ", " +
                std::to_string(pData[i].position.z) + ")");
    }
    pRenderContext->unmap(readbackVB);
}
```

**What this proves:**
- GPU has correct vertex data
- Memory transfer succeeded
- Data layout is correct

---

## Test 5: Test Custom Mesh Loading

**Add this test code:**

Create a test file `data/test_mesh.txt`:
```
4
0 1 0
1 -1 1
-1 -1 1
0 -1 -1
1
0 1 2 3
```

Then in code:
```cpp
// Test mesh loader
TetMesh customMesh = TetMesh::loadFromFile("data/test_mesh.txt");

if (customMesh.getVertexCount() == 4 && customMesh.getTetCount() == 1)
{
    logInfo("MESH LOADER: SUCCESS - loaded correct mesh");
}
else
{
    logError("MESH LOADER: FAILED - wrong vertex/tet count");
}
```

**What this proves:**
- File I/O works
- Parsing is correct
- Mesh importer is functional

---

## Test 6: Memory Leak Check

**Using Visual Studio:**
1. Build in Debug mode
2. Run app
3. Let it run for 30 seconds
4. Close app
5. Check Debug Output window for leak reports

**Expected:**
```
Detected memory leaks!
Dumping objects ->
{...}
Object dump complete.
```

Should show **0 bytes** leaked from our code (Falcor's pools are expected).

**What this proves:**
- No resource leaks in IntervalPass
- Buffers are properly freed
- ref<> pointers work correctly

---

## Test 7: Visual Verification

### Current State (Track 2):
- **Expected:** Solid blue window
- **Why:** `clearTexture(pIntervalOut, float4(0.2f, 0.5f, 0.8f, 1.0f))`

### After Track 3 Implements Compute Shader:
- **Expected:** Some visualization of tet data
- **View modes:** Front depth (R), Back depth (G), Interval length (B)
- **Test mesh:** Single pyramid should show depth gradient

---

## Test 8: Automated Validation

**Create a test function:**

```cpp
bool IntervalPass::validateTetMeshIntegrity()
{
    // Check 1: Vertex count
    if (mTetMesh.getVertexCount() != 4)
    {
        logError("VALIDATE FAIL: Expected 4 vertices, got " +
                 std::to_string(mTetMesh.getVertexCount()));
        return false;
    }

    // Check 2: Tet count
    if (mTetMesh.getTetCount() != 1)
    {
        logError("VALIDATE FAIL: Expected 1 tet, got " +
                 std::to_string(mTetMesh.getTetCount()));
        return false;
    }

    // Check 3: Index validity
    for (uint32_t i = 0; i < mTetMesh.getTetCount(); ++i)
    {
        for (uint32_t j = 0; j < 4; ++j)
        {
            uint32_t idx = mTetMesh.tetIndices[i * 4 + j];
            if (idx >= mTetMesh.getVertexCount())
            {
                logError("VALIDATE FAIL: Tet " + std::to_string(i) +
                         " has invalid index " + std::to_string(idx));
                return false;
            }
        }
    }

    // Check 4: GPU buffer existence
    if (!mpTetVertexBuffer || !mpTetIndexBuffer)
    {
        logError("VALIDATE FAIL: GPU buffers not created");
        return false;
    }

    logInfo("VALIDATE SUCCESS: All checks passed");
    return true;
}
```

Call it in `loadTetMesh()`:
```cpp
validateTetMeshIntegrity();
```

---

## Test 9: Performance Baseline

**Measure timing:**

```cpp
auto startTime = std::chrono::high_resolution_clock::now();

loadTetMesh(pRenderContext);

auto endTime = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

logInfo("loadTetMesh() took " + std::to_string(duration.count()) + " ms");
```

**Expected:**
- < 1 ms for single tet
- < 5 ms for 10k vertices
- < 50 ms for 1M vertices

---

## Test 10: Integration Test Checklist

- [ ] App launches without errors
- [ ] Blue window appears
- [ ] Console shows tet mesh info (4 vertices, 1 tet)
- [ ] GPU buffers created with correct sizes
- [ ] GPU memory contains correct vertex data (if using PIX)
- [ ] Custom mesh loads from .txt file
- [ ] No memory leaks on shutdown
- [ ] Debug validation passes
- [ ] Performance acceptable (< 1ms for single tet)

---

## Common Issues & Diagnostics

### Issue: No console output
**Check:**
- Is output window visible in VS? (Debug > Windows > Output)
- Verbosity level set to Info? (Falcor usually defaults to Info)

### Issue: Buffer creation fails silently
**Check:**
- Device is valid (not nullptr)
- Buffer sizes are > 0
- Bind flags are correct (ShaderResource for read-only)

### Issue: Indices out of range
**Check:**
- Tet indices are in [0, vertexCount)
- tetIndices.size() is multiple of 4
- Index validity in validation function

### Issue: GPU data doesn't match CPU data
**Check:**
- Data types align (TetVertex is 12 bytes = 3 floats)
- No padding/alignment issues
- Endianness correct (x86 is little-endian)

---

## Success Criteria

Your Track 2 implementation is correct if:

✅ **Structural:**
- TetMesh class properly encapsulates vertex and index data
- TetVertex has correct GPU layout (12 bytes)
- Hardcoded test tet has 4 vertices and 1 tet

✅ **GPU Upload:**
- Structured buffers created successfully
- Sizes match: 48 bytes (vertices) + 16 bytes (indices)
- Device memory allocated on GPU

✅ **Integration:**
- App runs without crashes
- Blue debug output visible
- Console logs show correct mesh info
- No memory leaks

✅ **Extensibility:**
- Mesh importer works with .txt files
- GPU buffers accessible to shaders (via variable names)
- Documentation complete for Track 3

---

## Next: Track 3 Testing

Once Track 3 implements the compute shader:

1. **Ray generation test:** Verify rays are computed correctly
2. **Ray-tet intersection:** Check against known test cases
3. **Interval output:** Visualize front/back depths with DebugViewPass
4. **Performance:** Profile compute shader on larger meshes

---

## Quick Debug Checklist

Before handing to Track 3, verify:

```cpp
// In loadTetMesh()

// ✓ CPU mesh created
assert(mTetMesh.getVertexCount() == 4);
assert(mTetMesh.getTetCount() == 1);

// ✓ GPU buffers allocated
assert(mpTetVertexBuffer != nullptr);
assert(mpTetIndexBuffer != nullptr);
assert(mpTetVertexBuffer->getSize() == 48);  // 4 * 12
assert(mpTetIndexBuffer->getSize() == 16);   // 4 * 4

// ✓ Data is readable from GPU (if using readback test)
// See Test 4 for readback code
```

All assertions passing = Track 2 is complete and correct! ✅
