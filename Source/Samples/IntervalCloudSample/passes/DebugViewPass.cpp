#include "DebugViewPass.h"

namespace
{
    const char* kIntervalIn = "intervalIn";
    const char* kColorOut = "color";
}

ref<DebugViewPass> DebugViewPass::create(ref<Device> pDevice)
{
    return ref<DebugViewPass>(new DebugViewPass(pDevice));
}

DebugViewPass::DebugViewPass(ref<Device> pDevice) : RenderPass(pDevice, "DebugViewPass")
{
    // TODO: Implement constructor
}

std::string DebugViewPass::getDesc()
{
    return "A render pass that visualizes interval data.";
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
        pRenderContext->clearFbo(pColorOut->getFbo().get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    }
}
