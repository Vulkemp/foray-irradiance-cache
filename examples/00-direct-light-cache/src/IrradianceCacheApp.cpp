#include "IrradianceCacheApp.h"

void irradiance_cache::IrradianceCacheApp::ApiInit() {
    mScene = std::make_unique<foray::scene::Scene>(&mContext);
    foray::gltf::ModelConverter converter(mScene.get());
    foray::gltf::ModelConverterOptions options{.FlipY = !INVERT_BLIT_INSTEAD};
    converter.LoadGltfModel(SCENE_FILE, nullptr, options);

    mScene->UpdateTlasManager();
    mScene->UseDefaultCamera(INVERT_BLIT_INSTEAD);
    mScene->UpdateLightManager();

    mRtStage.Init(&mContext, mScene.get());
    mSwapCopyStage.Init(&mContext, mRtStage.GetRtOutput());
    mSwapCopyStage.SetFlipY(INVERT_BLIT_INSTEAD);
    RegisterRenderStage(&mRtStage);
    RegisterRenderStage(&mSwapCopyStage);

    DefaultAppBase::ApiInit();
}

void irradiance_cache::IrradianceCacheApp::ApiOnResized(VkExtent2D size) {
    mScene->InvokeOnResized(size);
    DefaultAppBase::ApiOnResized(size);
}

void irradiance_cache::IrradianceCacheApp::ApiOnEvent(const foray::osi::Event *event) {
    mScene->InvokeOnEvent(event);
    DefaultAppBase::ApiOnEvent(event);
}

void irradiance_cache::IrradianceCacheApp::ApiRender(foray::base::FrameRenderInfo &renderInfo) {
    foray::core::DeviceSyncCommandBuffer& cmdBuffer = renderInfo.GetPrimaryCommandBuffer();
    cmdBuffer.Begin();
    renderInfo.GetInFlightFrame()->ClearSwapchainImage(cmdBuffer, renderInfo.GetImageLayoutCache());
    mScene->Update(renderInfo, cmdBuffer);
    mRtStage.RecordFrame(cmdBuffer, renderInfo);
    mSwapCopyStage.RecordFrame(cmdBuffer, renderInfo);
    renderInfo.GetInFlightFrame()->PrepareSwapchainImageForPresent(cmdBuffer, renderInfo.GetImageLayoutCache());
    cmdBuffer.Submit();
}

void irradiance_cache::IrradianceCacheApp::ApiDestroy()
{
    mRtStage.Destroy();
    mSwapCopyStage.Destroy();
    mScene = nullptr;
}
