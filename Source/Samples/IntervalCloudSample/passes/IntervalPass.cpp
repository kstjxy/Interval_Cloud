#include "IntervalPass.h"

namespace
{
    const char* kColorIn = "colorIn";
    const char* kIntervalOut = "intervalOut";
}

ref<IntervalPass> IntervalPass::create(ref<Device> pDevice, const Properties& props)
{
    return ref<IntervalPass>(new IntervalPass(pDevice, props));
}

IntervalPass::IntervalPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Track 2 reminder: once the tet loader lands, keep handles to the structured buffers here.
    // They'll be bound through ParameterBlocks so the mesh shader can emit intervals per pixel.
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

void IntervalPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    FALCOR_ASSERT(renderData[kColorIn] != nullptr, "IntervalPass missing color input");

    const auto& pIntervalOut = renderData.getTexture(kIntervalOut);
    if (pIntervalOut)
    {
        // Track 2 hook: this clear will be replaced with the mesh shader dispatch that writes (front, back) depth.
        // Leave the explicit clear for now so downstream passes always read defined values.
        pRenderContext->clearTexture(pIntervalOut.get());
    }
}
