#include "IntervalPass.h"
#include "Core/API/ComputeContext.h"

namespace
{
    const char* kColorIn = "colorIn";
    const char* kIntervalOut = "intervalOut";
    const char* kShaderFile = "Samples/IntervalCloudSample/passes/ComputeInterval.cs.slang";
}

ref<IntervalPass> IntervalPass::create(ref<Device> pDevice, const Properties& props)
{
    return ref<IntervalPass>(new IntervalPass(pDevice, props));
}

IntervalPass::IntervalPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Tet mesh will be loaded on first execute
}

RenderPassReflection IntervalPass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kColorIn, "Scene color buffer")
        .format(ResourceFormat::RGBA8UnormSrgb)
        .texture2D()
        .bindFlags(ResourceBindFlags::ShaderResource);
    reflector.addOutput(kIntervalOut, "Interval texture").format(ResourceFormat::RG16Float).bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
    return reflector;
}

void IntervalPass::loadTetMesh(RenderContext* pRenderContext)
{
    auto pDevice = mpDevice;

    // Create hardcoded single-tet mesh
    mTetMesh = TetMesh::createSingleTet();

    // Upload tet vertices to GPU
    // Create structured buffer with element size = sizeof(TetVertex)
    uint32_t vertexCount = mTetMesh.getVertexCount();
    uint32_t vertexSize = sizeof(TetVertex);
    mpTetVertexBuffer = pDevice->createStructuredBuffer(
        vertexCount,
        vertexSize,
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mTetMesh.vertices.data()
    );

    // Upload tet indices to GPU
    // Structured buffer: one uint32 per index (4 per tet)
    uint32_t indexCount = (uint32_t)mTetMesh.tetIndices.size();
    mpTetIndexBuffer = pDevice->createStructuredBuffer(
        indexCount,
        sizeof(uint32_t),
        ResourceBindFlags::ShaderResource,
        MemoryType::DeviceLocal,
        mTetMesh.tetIndices.data()
    );

    // Log mesh info
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

    // Verify GPU buffers
    logInfo("=== GPU BUFFERS ===");
    if (mpTetVertexBuffer)
    {
        uint64_t expectedSize = (uint64_t)mTetMesh.getVertexCount() * sizeof(TetVertex);
        uint64_t actualSize = mpTetVertexBuffer->getSize();
        logInfo("Vertex buffer: " + std::to_string(actualSize) + " bytes (expected " +
                std::to_string(expectedSize) + ")");
        if (actualSize == expectedSize)
            logInfo("  ✓ Vertex buffer size correct");
        else
            logError("  ✗ Vertex buffer size MISMATCH");
    }
    else
    {
        logError("Vertex buffer creation FAILED");
    }

    if (mpTetIndexBuffer)
    {
        uint64_t expectedSize = (uint64_t)mTetMesh.tetIndices.size() * sizeof(uint32_t);
        uint64_t actualSize = mpTetIndexBuffer->getSize();
        logInfo("Index buffer: " + std::to_string(actualSize) + " bytes (expected " +
                std::to_string(expectedSize) + ")");
        if (actualSize == expectedSize)
            logInfo("  ✓ Index buffer size correct");
        else
            logError("  ✗ Index buffer size MISMATCH");
    }
    else
    {
        logError("Index buffer creation FAILED");
    }

    logInfo("=== TRACK 2 COMPLETE ===");

    mbMeshLoaded = true;
}

void IntervalPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    FALCOR_ASSERT(renderData[kColorIn] != nullptr, "IntervalPass missing color input");

    // Get output texture (not const, since compute shader writes to it)
    ref<Texture> pIntervalOut = renderData.getTexture(kIntervalOut);
    if (!pIntervalOut)
        return;

    // Load tet mesh on first frame
    if (!mbMeshLoaded)
    {
        loadTetMesh(pRenderContext);
    }

    // For now: output debug gradient to verify pipeline is working
    // Track 3 will replace this with actual ray-tet intersection compute shader
    float4 debugColor(0.2f, 0.5f, 0.8f, 1.0f);  // Blue color
    pRenderContext->clearTexture(pIntervalOut.get(), debugColor);
}
