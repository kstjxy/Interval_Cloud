#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class DebugViewPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(DebugViewPass, "DebugViewPass", Falcor::RenderPass::PluginInfo{"A render pass that visualizes interval data."});

    static ref<DebugViewPass> create(ref<Device> pDevice, const Properties& props);
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

    void setViewMode(uint32_t viewMode) { mViewMode = viewMode; }

private:
    DebugViewPass(ref<Device> pDevice, const Properties& props);

    uint32_t mViewMode = 0;
};
