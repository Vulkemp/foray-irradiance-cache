#include "IrradianceCacheApp.h"

namespace foray::irradiance_cache {

    void IrradianceCacheApp::ApiInit() {
        mScene = std::make_unique<scene::Scene>(&mContext);
        gltf::ModelConverter converter(mScene.get());
        gltf::ModelConverterOptions options{.FlipY = !INVERT_BLIT_INSTEAD};
        converter.LoadGltfModel(SCENE_FILE, nullptr, options);

        mScene->UpdateTlasManager();
        mScene->UseDefaultCamera(INVERT_BLIT_INSTEAD);
        mScene->UpdateLightManager();

        // FIXME temp: guessed size of the scene
        const glm::vec3 dimensionsEstimate(20, 4, 20);
        glm::vec3 origin = -dimensionsEstimate;
        glm::vec3 extent = dimensionsEstimate * glm::vec3(2);
        const glm::vec3 probeDistance(0.2);
        VkExtent3D imageExtent = IrradianceCache::calculateImageExtend(extent, probeDistance);
        mIrradianceCache.emplace(&mContext, origin, extent, imageExtent);

        mIrradianceCacheFillStage.emplace(*mIrradianceCache, mScene.get());
        RegisterRenderStage(&*mIrradianceCacheFillStage);

        mRtStage.Init(&mContext, mScene.get());
        RegisterRenderStage(&mRtStage);

        mSwapCopyStage.Init(&mContext, mRtStage.GetRtOutput());
        mSwapCopyStage.SetFlipY(INVERT_BLIT_INSTEAD);
        RegisterRenderStage(&mSwapCopyStage);

        DefaultAppBase::ApiInit();
    }

    void IrradianceCacheApp::ApiOnResized(VkExtent2D size) {
        mScene->InvokeOnResized(size);
        DefaultAppBase::ApiOnResized(size);
    }

    void IrradianceCacheApp::ApiOnEvent(const osi::Event *event) {
        mScene->InvokeOnEvent(event);
        DefaultAppBase::ApiOnEvent(event);
    }

    void IrradianceCacheApp::ApiRender(base::FrameRenderInfo &renderInfo) {
        core::DeviceSyncCommandBuffer &cmdBuffer = renderInfo.GetPrimaryCommandBuffer();
        cmdBuffer.Begin();
        renderInfo.GetInFlightFrame()->ClearSwapchainImage(cmdBuffer, renderInfo.GetImageLayoutCache());
        mScene->Update(renderInfo, cmdBuffer);
        mIrradianceCacheFillStage->RecordFrame(cmdBuffer, renderInfo);
        mRtStage.RecordFrame(cmdBuffer, renderInfo);
        mSwapCopyStage.RecordFrame(cmdBuffer, renderInfo);
        renderInfo.GetInFlightFrame()->PrepareSwapchainImageForPresent(cmdBuffer, renderInfo.GetImageLayoutCache());
        cmdBuffer.Submit();
    }

    void IrradianceCacheApp::ApiDestroy() {
        mSwapCopyStage.Destroy();
        mRtStage.Destroy();
        mIrradianceCacheFillStage.reset();

        mIrradianceCache.reset();
        mScene = nullptr;
    }

}
