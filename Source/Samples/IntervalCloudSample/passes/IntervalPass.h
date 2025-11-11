#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

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
};
