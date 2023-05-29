#pragma once

#include <foray_api.hpp>
#include "FinalRTStage.h"
#include "IrradianceCacheDirectStage.h"
#include "IrrradianceCacheMode.h"
#include "IrradianceCacheIndirectStage.h"
#include "IrradianceCacheGui.h"

namespace foray::irradiance_cache {

    class IrradianceCacheApp : public foray::base::DefaultAppBase {
    protected:
        void ApiInit() override;

        void ApiOnResized(VkExtent2D size) override;

        void ApiOnEvent(const foray::osi::Event *event) override;

        void ApiRender(foray::base::FrameRenderInfo &renderInfo) override;

        void ApiDestroy() override;

    public:
        FORAY_GETTER_MR(Scene);

        FORAY_GETTER_MR(IrradianceCache);

        FORAY_PROPERTY_R(AllowSkipIC);

    private:
        std::unique_ptr<foray::scene::Scene> mScene;
        std::optional<IrradianceCache> mIrradianceCache;
        std::optional<IrradianceCacheGui> mGui;
        bool mAllowSkipIC = true;

        // stages
        std::optional<FinalRTStage> mRtStage;
        std::optional<IrradianceCacheDirectStage> mIrradianceCacheDirectStage;
        std::optional<IrradianceCacheIndirectStage> mIrradianceCacheIndirectStage;
        foray::stages::ImageToSwapchainStage mSwapCopyStage;
        foray::stages::ImguiStage mImguiStage;
    };

}
