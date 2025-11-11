#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"

using namespace Falcor;

class IntervalPass : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(IntervalPass, RenderPass);

    static ref<IntervalPass> create(ref<Device> pDevice);
    std::string getDesc() override;
    RenderPassReflection reflect(const CompileData& compileData) override;
    void execute(RenderContext* pRenderContext, const RenderData& renderData) override;

private:
    IntervalPass(ref<Device> pDevice);
};
