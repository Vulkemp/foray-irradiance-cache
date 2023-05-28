#include "IrradianceCacheApp.h"
#include "scene/globalcomponents/foray_aabbmanager.hpp"
#include <imgui/imgui.h>

namespace foray::irradiance_cache {

    const std::string SCENE_FILE = DATA_DIR "/gltf/CornellBoxPointLight/CornellBoxPointLight.gltf";
    const glm::vec3 PROBE_DISTANCE(0.05);

//    const std::string SCENE_FILE = DATA_DIR "/gltf/testbox/scene.gltf";
//    const glm::vec3 PROBE_DISTANCE(0.2);

    /// @brief If true, will invert the viewport when blitting. Will invert the scene while loading to -Y up if false
    constexpr bool INVERT_BLIT_INSTEAD = true;

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

        mImguiStage.InitForSwapchain(&mContext);
        mImguiStage.AddWindowDraw([this]() { this->ImGui(); });
        RegisterRenderStage(&mImguiStage);

        DefaultAppBase::ApiInit();
    }

    void IrradianceCacheApp::ApiOnResized(VkExtent2D size) {
        mScene->InvokeOnResized(size);
        DefaultAppBase::ApiOnResized(size);
    }

    void IrradianceCacheApp::ApiOnEvent(const osi::Event *event) {
        mScene->InvokeOnEvent(event);
        mImguiStage.ProcessSdlEvent(&(event->RawSdlEventData));
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
        mImguiStage.RecordFrame(cmdBuffer, renderInfo);
        renderInfo.GetInFlightFrame()->PrepareSwapchainImageForPresent(cmdBuffer, renderInfo.GetImageLayoutCache());
        cmdBuffer.Submit();
        mIrradianceCache->frameFinished();
    }

    void IrradianceCacheApp::ImGui() {
        ImGui::Begin("window");
        foray::base::RenderLoop::FrameTimeAnalysis analysis = mRenderLoop.AnalyseFrameTimes();
        ImGui::Text("FPS: %f avg %f min", analysis.Count > 0 ? 1.f / analysis.AvgFrameTime : 0, analysis.Count > 0 ? 1.f / analysis.MaxFrameTime : 0);

        std::string modeName(NAMEOF_ENUM(mIrradianceCache->GetMode()));
        if (ImGui::BeginCombo("Mode", modeName.c_str())) {
            for (uint32_t i = 0; i < (uint32_t) IrradianceCacheMode::MAX_ENUM; i++) {
                auto mode = (IrradianceCacheMode) i;
                modeName = std::string(NAMEOF_ENUM(mode));
                if (ImGui::MenuItem(modeName.c_str())) {
                    mIrradianceCache->SetMode(mode);
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Clear Irradiance Cache")) {
            mIrradianceCache->clearCache();
        }

        ImGui::End();
    }

    void IrradianceCacheApp::ApiDestroy() {
        mImguiStage.Destroy();
        mSwapCopyStage.Destroy();
        mRtStage.reset();
        mIrradianceCacheIndirectStage.reset();
        mIrradianceCacheDirectStage.reset();

        mIrradianceCache.reset();
        mScene = nullptr;
    }

}
