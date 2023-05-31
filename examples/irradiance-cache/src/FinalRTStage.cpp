#include "FinalRTStage.h"
#include "scene/foray_scene.hpp"

namespace foray::irradiance_cache {

    // bindpoints
    const uint32_t BIND_LIGHTS = 11;
    const uint32_t BIND_IN_IRRADIANCE_CACHE = 12;

    // shaders
    const std::string FOLDER_IRRADIANCE_CACHE_DIRECT = "shaders/finalrt/";
    const std::string RAYGEN_FILE = FOLDER_IRRADIANCE_CACHE_DIRECT + "raygen.rgen";

    FinalRTShaders::FinalRTShaders(FinalRTStage &s) : mStage(s) {
        foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR, EXAMPLE_SHADER_DIR}};
        mVisiTest.emplace(s.mContext, options, 1);
        mProbeMat.emplace(s.mContext, options, 0);

        s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));
        s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
        mProbeMat->Compile(s.mContext, options, s.mShaderKeys, s.mPipeline);
        mVisiTest->Compile(s.mContext, options, s.mShaderKeys, s.mPipeline);
        s.mPipeline.Build(s.mContext, s.mPipelineLayout);
    }

    FinalRTShaders::~FinalRTShaders() {
        mStage.mPipeline.Destroy();
    }

    void FinalRTStage::ApiCreateRtPipeline() {
        mShaders.emplace(*this);
    }

    void FinalRTStage::ApiDestroyRtPipeline() {
        mShaders.reset();
    }

    // stage
    FinalRTStage::FinalRTStage(IrradianceCache &mIrradianceCache, foray::core::Context *context, foray::scene::Scene *scene) : mIrradianceCache(mIrradianceCache) {
        // disable push constant
        mRngSeedPushCOffset = ~0;

        mLightManager = scene->GetComponent<foray::scene::gcomp::LightManager>();
        foray::stages::DefaultRaytracingStageBase::Init(context, scene);
    }

    void FinalRTStage::CreateOrUpdateDescriptors() {
        mDescriptorSet.SetDescriptorAt(BIND_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(),
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, foray::stages::RTSTAGEFLAGS);
        mDescriptorSet.SetDescriptorAt(BIND_IN_IRRADIANCE_CACHE, mIrradianceCache.GetIndirectImage(), VK_IMAGE_LAYOUT_GENERAL,
                                       nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stages::RTSTAGEFLAGS);
        DefaultRaytracingStageBase::CreateOrUpdateDescriptors();
    }

    void FinalRTStage::CreatePipelineLayout() {
        mIrradianceCacheShaderAccess.emplace(mIrradianceCache, mPipelineLayout, stages::RTSTAGEFLAGS);
        DefaultRaytracingStageBase::CreatePipelineLayout();
    }

    void FinalRTStage::RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo, std::vector<VkImageMemoryBarrier2> &imageFullBarriers,
                                           std::vector<VkImageMemoryBarrier2> &imageByRegionBarriers, std::vector<VkBufferMemoryBarrier2> &bufferBarriers) {
        DefaultRaytracingStageBase::RecordFrameBarriers(cmdBuffer, renderInfo, imageFullBarriers, imageByRegionBarriers, bufferBarriers);

        // full barrier as IC stages may not run
        imageFullBarriers.push_back(renderInfo.GetImageLayoutCache().MakeBarrier(mIrradianceCache.GetIndirectImage(), core::ImageLayoutCache::Barrier2{
                .SrcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .SrcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                .DstStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .DstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
                .NewLayout     = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL
        }));
    }

    void FinalRTStage::RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        DefaultRaytracingStageBase::RecordFrameBind(cmdBuffer, renderInfo);
        mIrradianceCacheShaderAccess->pushConstants(cmdBuffer, renderInfo);
    }

}
