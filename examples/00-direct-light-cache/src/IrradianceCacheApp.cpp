#include "IrradianceCacheApp.h"
#include "scene/globalcomponents/foray_aabbmanager.hpp"

namespace foray::irradiance_cache {

    const std::string SCENE_FILE = DATA_DIR "/gltf/testbox/scene.gltf";
    /// @brief If true, will invert the viewport when blitting. Will invert the scene while loading to -Y up if false
    constexpr bool INVERT_BLIT_INSTEAD = true;
    const glm::vec3 PROBE_DISTANCE(0.2);

    void IrradianceCacheApp::ApiInit() {
        mScene = std::make_unique<scene::Scene>(&mContext);
        gltf::ModelConverter converter(mScene.get());
        gltf::ModelConverterOptions options{.FlipY = !INVERT_BLIT_INSTEAD};
        converter.LoadGltfModel(SCENE_FILE, nullptr, options);

        mScene->UpdateTlasManager();
        mScene->UseDefaultCamera(INVERT_BLIT_INSTEAD);
        mScene->UpdateLightManager();

        auto *aabbManager = mScene->MakeComponent<foray::scene::gcomp::AabbManager>();
        aabbManager->CompileAabbs();
        glm::vec3 origin = aabbManager->GetMinBounds();
        glm::vec3 extent = aabbManager->GetMaxBounds() - origin;
        VkExtent3D imageExtent = IrradianceCache::calculateImageExtend(extent, PROBE_DISTANCE);
        foray::logger()->info("IrradianceCache: dimensions ({}, {}, {})", imageExtent.width, imageExtent.height, imageExtent.depth);
        foray::logger()->info("IrradianceCache: worldspace ({}, {}, {}) +({}, {}, {})", origin.x, origin.y, origin.z, extent.x, extent.y, extent.z);
        mIrradianceCache.emplace(&mContext, origin, extent, imageExtent);

        mIrradianceCacheDirectStage.emplace(*mIrradianceCache, mScene.get());
        RegisterRenderStage(&*mIrradianceCacheDirectStage);

        mIrradianceCacheIndirectStage.emplace(*mIrradianceCache, mScene.get());
        RegisterRenderStage(&*mIrradianceCacheIndirectStage);

        mRtStage.emplace(*mIrradianceCache, &mContext, mScene.get());
        RegisterRenderStage(&*mRtStage);

        mSwapCopyStage.Init(&mContext, mRtStage->GetRtOutput());
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

        if (event->Type == osi::Event::EType::InputBinary) {
            auto *e = reinterpret_cast<const osi::EventInputBinary *>(event);
            if (e->State) {
                auto mode = (int32_t) mIrradianceCache->GetMode();
                if (e->SourceInput->GetButtonId() == foray::osi::EButton::Keyboard_T) {
                    mode--;
                } else if (e->SourceInput->GetButtonId() == foray::osi::EButton::Keyboard_G) {
                    mode++;
                }

                if (mode < 0) {
                    mode = (int32_t) IrradianceCacheMode::MAX_ENUM - 1;
                } else if (mode > (int32_t) IrradianceCacheMode::MAX_ENUM - 1) {
                    mode = 0;
                }
                mIrradianceCache->SetMode((IrradianceCacheMode) mode);

                if (e->SourceInput->GetButtonId() == foray::osi::EButton::Keyboard_C) {
                    mIrradianceCache->clearCache();
                }
            }
        }
    }

    void IrradianceCacheApp::ApiRender(base::FrameRenderInfo &renderInfo) {
        core::DeviceSyncCommandBuffer &cmdBuffer = renderInfo.GetPrimaryCommandBuffer();
        cmdBuffer.Begin();
        renderInfo.GetInFlightFrame()->ClearSwapchainImage(cmdBuffer, renderInfo.GetImageLayoutCache());
        mScene->Update(renderInfo, cmdBuffer);
        mIrradianceCacheDirectStage->RecordFrame(cmdBuffer, renderInfo);
        mIrradianceCacheIndirectStage->RecordFrame(cmdBuffer, renderInfo);
        mRtStage->RecordFrame(cmdBuffer, renderInfo);
        mSwapCopyStage.RecordFrame(cmdBuffer, renderInfo);
        renderInfo.GetInFlightFrame()->PrepareSwapchainImageForPresent(cmdBuffer, renderInfo.GetImageLayoutCache());
        cmdBuffer.Submit();
    }

    void IrradianceCacheApp::ApiDestroy() {
        mSwapCopyStage.Destroy();
        mRtStage.reset();
        mIrradianceCacheIndirectStage.reset();
        mIrradianceCacheDirectStage.reset();

        mIrradianceCache.reset();
        mScene = nullptr;
    }

}
