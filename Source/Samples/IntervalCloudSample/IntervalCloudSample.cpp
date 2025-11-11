/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#include "IntervalCloudSample.h"
#include "RenderGraph/RenderGraph.h" // Added for RenderGraph

FALCOR_EXPORT_D3D12_AGILITY_SDK

uint32_t mSampleGuiWidth = 250;
uint32_t mSampleGuiHeight = 200;
uint32_t mSampleGuiPositionX = 20;
uint32_t mSampleGuiPositionY = 40;

IntervalCloudSample::IntervalCloudSample(const SampleAppConfig& config) : SampleApp(config)
{
    //
}

IntervalCloudSample::~IntervalCloudSample()
{
    //
}

void IntervalCloudSample::onLoad(RenderContext* pRenderContext)
{
    mpRenderGraph = RenderGraph::create(pRenderContext->getDevice(), "IntervalGraph");

    // Track 2 note: this is where GPU buffers (tet vertices + indices) will be created/attached once the loader exists.
    // For now we just stand up the pass objects so the render graph plumbing is ready for real data.
    mpClearPass = ClearPass::create(pRenderContext->getDevice(), {});
    mpIntervalPass = IntervalPass::create(pRenderContext->getDevice(), {});
    mpDebugViewPass = DebugViewPass::create(pRenderContext->getDevice(), {});

    // Add passes to graph
    mpRenderGraph->addPass(mpClearPass, "Clear");
    mpRenderGraph->addPass(mpIntervalPass, "Interval");
    mpRenderGraph->addPass(mpDebugViewPass, "Debug");

    // Connect passes
    mpRenderGraph->addEdge("Clear.color", "Interval.colorIn");
    mpRenderGraph->addEdge("Interval.intervalOut", "Debug.intervalIn");
    mpRenderGraph->markOutput("Debug.color");

    mpRenderGraph->onResize(getTargetFbo().get());

    // Set the graph on the framework
    // Removed setRenderGraph(mpRenderGraph); as it's not a direct member of SampleApp
}

void IntervalCloudSample::onShutdown()
{
    //
}

void IntervalCloudSample::onResize(uint32_t width, uint32_t height)
{
    // TODO: Handle resizing of resources. For now, Falcor handles it via the graph.
    if (mpRenderGraph && getTargetFbo())
    {
        mpRenderGraph->onResize(getTargetFbo().get());
    }
}

void IntervalCloudSample::onFrameRender(RenderContext* pRenderContext, const ref<Fbo>& pTargetFbo)
{
    if (!mpRenderGraph) return;

    mpRenderGraph->execute(pRenderContext); // Track 2: once the mesh path lands, IntervalPass will consume SRVs before Debug paints them.

    auto pOutput = mpRenderGraph->getOutput("Debug.color");
    auto pDstView = pTargetFbo ? pTargetFbo->getRenderTargetView(0) : nullptr;
    if (!pOutput || !pDstView) return;

    if (auto pTex = pOutput->asTexture())
    {
        if (auto pSrcView = pTex->getSRV())
        {
            pRenderContext->blit(pSrcView, pDstView);
        }
    }
}

void IntervalCloudSample::onGuiRender(Gui* pGui)
{
    Gui::Window w(pGui, "Falcor", {250, 200});
    renderGlobalUI(pGui);
    
    // View Mode controls
    w.text("View Mode:");
    const Falcor::Gui::RadioButtonGroup viewModeButtons = {
        {(uint32_t)ViewMode::Front, "Front", false},
        {(uint32_t)ViewMode::Back, "Back", false},
        {(uint32_t)ViewMode::Length, "Length", false}
    };
    if (w.radioButtons(viewModeButtons, (uint32_t&)mViewMode))
    {
        if (mpDebugViewPass) mpDebugViewPass->setViewMode((uint32_t)mViewMode);
    }
    w.text("Current Mode: " + std::string(mViewMode == ViewMode::Front ? "Front" : (mViewMode == ViewMode::Back ? "Back" : "Length")));

    // Original SampleAppTemplate controls (optional, can be removed if not needed)
    w.text("Hello from IntervalCloudSample");
    if (w.button("Click Here"))
    {
        msgBox("Info", "Now why would you do that?");
    }
}

bool IntervalCloudSample::onKeyEvent(const KeyboardEvent& keyEvent)
{
    return false;
}

bool IntervalCloudSample::onMouseEvent(const MouseEvent& mouseEvent)
{
    return false;
}

void IntervalCloudSample::onHotReload(HotReloadFlags reloaded)
{
    //
}

int runMain(int argc, char** argv)
{
    SampleAppConfig config;
    config.windowDesc.title = "Interval Cloud Sample";
    config.windowDesc.resizableWindow = true;

    IntervalCloudSample project(config);
    return project.run();
}

int main(int argc, char** argv)
{
    return catchAndReportAllExceptions([&]() { return runMain(argc, argv); });
}
