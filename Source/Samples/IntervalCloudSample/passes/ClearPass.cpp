#include "ClearPass.h"

namespace
{
    const char* kColorOut = "color";
}

ref<ClearPass> ClearPass::create(ref<Device> pDevice, const Properties& props)
{
    return ref<ClearPass>(new ClearPass(pDevice, props));
}

ClearPass::ClearPass(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    (void)props;
}

RenderPassReflection ClearPass::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    reflector.addOutput(kColorOut, "Color output")
        .format(ResourceFormat::RGBA8UnormSrgb)
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
    return reflector;
}

void ClearPass::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    const auto& pColor = renderData.getTexture(kColorOut);
    if (pColor)
    {
        // Keep a predictable background so interval debug colors are obvious while Track 2 work lands.
        pRenderContext->clearTexture(pColor.get(), mClearColor);
    }
}
