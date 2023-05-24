#pragma once

#include <foray_api.hpp>
#include "FinalRTStage.h"
#include "IrradianceCacheDirectStage.h"
#include "IrrradianceCacheMode.h"
#include "IrradianceCacheIndirectStage.h"

namespace foray::irradiance_cache {

    class IrradianceCacheApp : public foray::base::DefaultAppBase {
    protected:
        void ApiInit() override;

        void ApiOnResized(VkExtent2D size) override;

        void ApiOnEvent(const foray::osi::Event *event) override;

        void ApiRender(foray::base::FrameRenderInfo &renderInfo) override;

        void ApiDestroy() override;

        void ImGui();

    private:
        std::unique_ptr<foray::scene::Scene> mScene;
        std::optional<IrradianceCache> mIrradianceCache;

        // stages
        std::optional<FinalRTStage> mRtStage;
        std::optional<IrradianceCacheDirectStage> mIrradianceCacheDirectStage;
        std::optional<IrradianceCacheIndirectStage> mIrradianceCacheIndirectStage;
        foray::stages::ImageToSwapchainStage mSwapCopyStage;
        foray::stages::ImguiStage mImguiStage;
    };

}
