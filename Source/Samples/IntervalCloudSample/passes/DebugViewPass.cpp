#include "DebugViewPass.h"

namespace
{
    const char* kIntervalIn = "intervalIn";
    const char* kColorOut = "color";
}

ref<DebugViewPass> DebugViewPass::create(ref<Device> pDevice, const Properties& props)
{
    return ref<DebugViewPass>(new DebugViewPass(pDevice, props));
}

DebugViewPass::DebugViewPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Track 2: when interval textures hold real data, this pass will pull structured values
    // (front/back/length) and shade quads accordingly instead of clearing to a flat color.
}

RenderPassReflection DebugViewPass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addInput(kIntervalIn, "Input interval texture").format(ResourceFormat::RG16Float);
    reflector.addOutput(kColorOut, "Output color").format(ResourceFormat::RGBA8UnormSrgb).bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
    return reflector;
}

void DebugViewPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pIntervalIn = renderData.getTexture(kIntervalIn);
    const auto& pColorOut = renderData.getTexture(kColorOut);

    if (pIntervalIn && pColorOut)
    {
        float4 clearColor;
        if (mViewMode == 0) // Front
        {
            clearColor = float4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
        }
        else if (mViewMode == 1) // Back
        {
            clearColor = float4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        }
        else // Length
        {
            clearColor = float4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
        }
        // Track 2: replace clearTexture() with a fullscreen pass that visualizes the decoded interval SLAB.
        // The solid colors make it obvious that GUI wiring works even before shaders are in place.
        pRenderContext->clearTexture(pColorOut.get(), clearColor);
    }
}
