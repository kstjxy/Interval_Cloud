#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class DebugViewPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(DebugViewPass, RenderPass);

    static ref<DebugViewPass> create(ref<Device> pDevice);
    std::string getDesc() override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

    void setViewMode(uint32_t viewMode) { mViewMode = viewMode; }

private:
    DebugViewPass(ref<Device> pDevice);

    uint32_t mViewMode = 0;
};
