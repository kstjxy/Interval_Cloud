#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "../TetMesh.h"

using namespace Falcor;

class IntervalPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(IntervalPass, "IntervalPass", Falcor::RenderPass::PluginInfo{"A render pass that produces an interval texture."});

    static ref<IntervalPass> create(ref<Device> pDevice, const Properties& props);
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

private:
    IntervalPass(ref<Device> pDevice, const Properties& props);

    // Tet mesh data
    TetMesh mTetMesh;
    ref<Buffer> mpTetVertexBuffer;
    ref<Buffer> mpTetIndexBuffer;

    // Compute pass for interval computation
    ref<ComputePass> mpComputePass;
    ref<ProgramVars> mpVars;

    // Initialization flag
    bool mbMeshLoaded = false;

    // Helper: load/initialize the tet mesh and GPU buffers
    void loadTetMesh(RenderContext* pRenderContext);
};
