#pragma once

#include "stages/foray_defaultraytracingstage.hpp"
#include "scene/globalcomponents/foray_lightmanager.hpp"
#include "core/foray_samplercollection.hpp"
#include "VisiTest.h"
#include "IrradianceCache.h"
#include "ProbeMat.h"

namespace foray::irradiance_cache {

    class FinalRTStage;

    class FinalRTShaders {
    public:
        explicit FinalRTShaders(FinalRTStage &s);

        ~FinalRTShaders();

    private:
        FinalRTStage &mStage;
        core::ShaderModule mRaygen;
        std::optional<ProbeMat> mProbeMat;
        std::optional<VisiTest> mVisiTest;
    };

    class FinalRTStage : public stages::DefaultRaytracingStageBase {
        friend FinalRTShaders;

    public:
        FinalRTStage(IrradianceCache &mIrradianceCache, foray::core::Context *context, foray::scene::Scene *scene);

    protected:
        void ApiCreateRtPipeline() override;

        void ApiDestroyRtPipeline() override;

        void CreateOrUpdateDescriptors() override;

        void CreatePipelineLayout() override;

        void RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo, std::vector<VkImageMemoryBarrier2> &imageFullBarriers,
                                 std::vector<VkImageMemoryBarrier2> &imageByRegionBarriers, std::vector<VkBufferMemoryBarrier2> &bufferBarriers) override;

        void RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) override;

    private:
        std::optional<FinalRTShaders> mShaders;
        scene::gcomp::LightManager *mLightManager;
        IrradianceCache &mIrradianceCache;
        std::optional<IrradianceCacheShaderAccess> mIrradianceCacheShaderAccess;
    };

}

