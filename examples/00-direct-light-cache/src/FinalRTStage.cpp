#include "FinalRTStage.h"
#include "scene/foray_scene.hpp"

namespace foray::irradiance_cache {

    // shaders
    const std::string FOLDER_IRRADIANCE_CACHE = "shaders/finalrt/";
    const std::string RAYGEN_FILE = FOLDER_IRRADIANCE_CACHE + "raygen.rgen";
    const std::string CLOSESTHIT_FILE = FOLDER_IRRADIANCE_CACHE + "closesthit.rchit";
    const std::string ANYHIT_FILE = FOLDER_IRRADIANCE_CACHE + "anyhit.rahit";
    const std::string MISS_FILE = FOLDER_IRRADIANCE_CACHE + "miss.rmiss";

    FinalRTShaders::FinalRTShaders(FinalRTStage &s) :
            mStage(s) {
        foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR}};
        s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));
        s.mShaderKeys.push_back(mClosestHit.CompileFromSource(s.mContext, CLOSESTHIT_FILE, options));
        s.mShaderKeys.push_back(mAnyHit.CompileFromSource(s.mContext, ANYHIT_FILE, options));
        s.mShaderKeys.push_back(mMiss.CompileFromSource(s.mContext, MISS_FILE, options));

        s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
        s.mPipeline.GetHitSbt().SetGroup(0, &mClosestHit, &mAnyHit, nullptr);
        s.mPipeline.GetMissSbt().SetGroup(0, &mMiss);
        mVisiTest.Init(s.mContext, s.mShaderKeys, s.mPipeline, 1);

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
        mIrradianceCacheSampler = core::Combined3dImageSampler(context, &mIrradianceCache.GetImage(), {
                .sType                   = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter               = VkFilter::VK_FILTER_LINEAR,
                .minFilter               = VkFilter::VK_FILTER_LINEAR,
                .mipmapMode              = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU            = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                .addressModeV            = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                .addressModeW            = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                .mipLodBias              = 0.f,
                .anisotropyEnable        = VK_FALSE,
                .maxAnisotropy           = 0,
                .compareEnable           = VK_FALSE,
                .compareOp               = {},
                .minLod                  = 0,
                .maxLod                  = VK_LOD_CLAMP_NONE,
                .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
                .unnormalizedCoordinates = VK_FALSE
        });
        foray::stages::DefaultRaytracingStageBase::Init(context, scene);
    }

    void FinalRTStage::CreateOrUpdateDescriptors() {
        mDescriptorSet.SetDescriptorAt(BINDPOINT_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(),
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, foray::stages::RTSTAGEFLAGS);
        mDescriptorSet.SetDescriptorAt(BIND_IN_IRRADIANCE_CACHE, mIrradianceCache.GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mIrradianceCacheSampler,
                                       VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stages::RTSTAGEFLAGS);
        DefaultRaytracingStageBase::CreateOrUpdateDescriptors();
    }

    void FinalRTStage::CreatePipelineLayout() {
        mIrradianceCacheShaderAccess.emplace(mIrradianceCache, mPipelineLayout, stages::RTSTAGEFLAGS);
        DefaultRaytracingStageBase::CreatePipelineLayout();
    }

    void FinalRTStage::RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo, std::vector<VkImageMemoryBarrier2> &imageBarriers,
                                           std::vector<VkBufferMemoryBarrier2> &bufferBarriers) {
        DefaultRaytracingStageBase::RecordFrameBarriers(cmdBuffer, renderInfo, imageBarriers, bufferBarriers);

        imageBarriers.push_back(renderInfo.GetImageLayoutCache().MakeBarrier(mIrradianceCache.GetImage(), core::ImageLayoutCache::Barrier2{
                .SrcStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .SrcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .DstStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .DstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
                .NewLayout     = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        }));
    }

    void FinalRTStage::RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        DefaultRaytracingStageBase::RecordFrameBind(cmdBuffer, renderInfo);
        mIrradianceCacheShaderAccess->pushConstants(cmdBuffer, renderInfo);
    }

}
