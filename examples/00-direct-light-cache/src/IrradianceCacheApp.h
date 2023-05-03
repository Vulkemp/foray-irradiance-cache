#pragma once

#include <foray_api.hpp>
#include "FinalRTStage.h"
#include "IrradianceCacheFillStage.hpp"

namespace foray::irradiance_cache {

    inline const std::string SCENE_FILE = DATA_DIR "/gltf/testbox/scene.gltf";
    /// @brief If true, will invert the viewport when blitting. Will invert the scene while loading to -Y up if false
    inline constexpr bool INVERT_BLIT_INSTEAD = true;

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
