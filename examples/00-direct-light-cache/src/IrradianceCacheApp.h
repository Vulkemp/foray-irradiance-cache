#pragma once

#include <foray_api.hpp>
#include "FinalRTStage.h"
#include "IrradianceCacheFillStage.hpp"

namespace foray::irradiance_cache {

    class IrradianceCacheApp : public foray::base::DefaultAppBase {
    protected:
        void ApiInit() override;

        void ApiOnResized(VkExtent2D size) override;

        void ApiOnEvent(const foray::osi::Event *event) override;

        void ApiRender(foray::base::FrameRenderInfo &renderInfo) override;

        void ApiDestroy() override;

    private:
        std::unique_ptr<foray::scene::Scene> mScene;
        std::optional<IrradianceCache> mIrradianceCache;

        // stages
        std::optional<FinalRTStage> mRtStage;
        std::optional<IrradianceCacheFillStage> mIrradianceCacheFillStage;
        foray::stages::ImageToSwapchainStage mSwapCopyStage;
    };

}
