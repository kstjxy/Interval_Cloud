#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class ClearPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(ClearPass, "IntervalClearPass", Falcor::RenderPass::PluginInfo{"Clears the main color buffer."});

    static ref<ClearPass> create(ref<Device> pDevice, const Properties& props);
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

    void setColor(const float4& color) { mClearColor = color; }

private:
    ClearPass(ref<Device> pDevice, const Properties& props);

    float4 mClearColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
};
