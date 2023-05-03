#pragma once

#include "stages/foray_raytracingstage.hpp"
#include "IrradianceCache.h"
#include "VisiTest.h"
#include "scene/globalcomponents/foray_lightmanager.hpp"

namespace foray::irradiance_cache {

    class IrradianceCacheFillStage;

    class IrradianceCacheFillShaders {
    public:
        explicit IrradianceCacheFillShaders(IrradianceCacheFillStage &s);

        ~IrradianceCacheFillShaders();

    private:
        IrradianceCacheFillStage &mStage;
        core::ShaderModule mRaygen;
        VisiTest mVisiTest;
    };

    class IrradianceCacheFillStage : public stages::RaytracingStageBase {
        friend IrradianceCacheFillShaders;

    public:
        IrradianceCacheFillStage(IrradianceCache &irradianceCache, scene::Scene *scene, core::CombinedImageSampler *envMap = nullptr,
                                 core::ManagedImage *noiseImage = nullptr);

        FORAY_GETTER_R(IrradianceCache);

    protected:
        void CreateOutputImages() override;

        void ApiCreateRtPipeline() override;

        void ApiDestroyRtPipeline() override;

        void RecordFrameTraceRays(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) override;

        void RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) override;

        void CreatePipelineLayout() override;

        void CreateOrUpdateDescriptors() override;

        void RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo, std::vector<VkImageMemoryBarrier2> &imageBarriers,
                                 std::vector<VkBufferMemoryBarrier2> &bufferBarriers) override;

        scene::gcomp::LightManager *mLightManager;
        IrradianceCache &mIrradianceCache;
        std::optional<IrradianceCacheShaderAccess> mIrradianceCacheShaderAccess;
        std::optional<IrradianceCacheFillShaders> mShaders;
    };
}
