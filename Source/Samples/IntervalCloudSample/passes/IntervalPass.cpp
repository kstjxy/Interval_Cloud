#include "IntervalPass.h"

namespace
{
    const char* kIntervalOut = "intervalOut";
}

ref<IntervalPass> IntervalPass::create(ref<Device> pDevice)
{
    return ref<IntervalPass>(new IntervalPass(pDevice));
}

IntervalPass::IntervalPass(ref<Device> pDevice) : RenderPass(pDevice, "IntervalPass")
{
    // TODO: Implement constructor
}

std::string IntervalPass::getDesc()
{
    return "A render pass that produces an interval texture.";
}

RenderPassReflection IntervalPass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addOutput(kIntervalOut, "Interval texture").format(ResourceFormat::RG16Float).bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
    return reflector;
}

void IntervalPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pIntervalOut = renderData.getTexture(kIntervalOut);
    if (pIntervalOut)
    {
        pRenderContext->clearTexture(pIntervalOut.get());
    }
}
