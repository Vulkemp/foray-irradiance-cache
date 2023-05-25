#include "IrradianceCacheIndirectStage.h"
#include "core/foray_samplercollection.hpp"
#include "scene/foray_scene.hpp"
#include "util/foray_shaderstagecreateinfos.hpp"

namespace foray::irradiance_cache {

    // bindpoints
    const uint32_t BIND_LIGHTS = 11;
    const uint32_t BIND_IN_IRRADIANCE_CACHE = 12;

    // shaders
    const std::string FOLDER_IRRADIANCE_CACHE_INDIRECT = "shaders/irradiancecache/indirect/";
    const std::string RAYGEN_FILE = FOLDER_IRRADIANCE_CACHE_INDIRECT + "raygen.rgen";

    IrradianceCacheIndirectShaders::IrradianceCacheIndirectShaders(IrradianceCacheIndirectStage &s) : mStage(s) {
        foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR, EXAMPLE_SHADER_DIR}};
        mProbeMat.emplace(s.mContext, options, 0);
        mVisiTest.emplace(s.mContext, options, 1);

        s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));
        s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
        mProbeMat->Compile(s.mContext, options, s.mShaderKeys, s.mPipeline);
        mVisiTest->Compile(s.mContext, options, s.mShaderKeys, s.mPipeline);
        s.mPipeline.Build(s.mContext, s.mPipelineLayout);
    }

    IrradianceCacheIndirectShaders::~IrradianceCacheIndirectShaders() {
        mStage.mPipeline.Destroy();
    }

    void IrradianceCacheIndirectStage::ApiCreateRtPipeline() {
        mShaders.emplace(*this);
    }

    void IrradianceCacheIndirectStage::ApiDestroyRtPipeline() {
        mShaders.reset();
    }

    // stage
    IrradianceCacheIndirectStage::IrradianceCacheIndirectStage(IrradianceCache &irradianceCache, scene::Scene *scene, core::CombinedImageSampler *envMap,
                                                               core::ManagedImage *noiseImage) :
            mIrradianceCache(irradianceCache) {
        // disable push constant
        mRngSeedPushCOffset = ~0;

        mLightManager = scene->GetComponent<foray::scene::gcomp::LightManager>();
        Init(irradianceCache.GetContext(), scene, envMap, noiseImage);
    }

    void IrradianceCacheIndirectStage::CreateOutputImages() {
        // IrradianceCache is managed separately from the FillStage
        // also prevents it from being recreated when swapchain size changes
    }

    void IrradianceCacheIndirectStage::CreateOrUpdateDescriptors() {
        using namespace stages::rtbindpoints;

        mDescriptorSet.SetDescriptorAt(BIND_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(),
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, foray::stages::RTSTAGEFLAGS);
        mDescriptorSet.SetDescriptorAt(BIND_IN_IRRADIANCE_CACHE, mIrradianceCache.GetTempImage(), VK_IMAGE_LAYOUT_GENERAL,
                                       nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stages::RTSTAGEFLAGS);
        mDescriptorSet.SetDescriptorAt(BIND_OUT_IMAGE, mIrradianceCache.GetIndirectImage(), VK_IMAGE_LAYOUT_GENERAL,
                                       nullptr, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, stages::RTSTAGEFLAGS);
        RaytracingStageBase::CreateOrUpdateDescriptors();
    }

    void IrradianceCacheIndirectStage::CreatePipelineLayout() {
        mIrradianceCacheShaderAccess.emplace(mIrradianceCache, mPipelineLayout, stages::RTSTAGEFLAGS);
        RaytracingStageBase::CreatePipelineLayout();
    }

    void IrradianceCacheIndirectStage::RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo,
                                                           std::vector<VkImageMemoryBarrier2> &imageFullBarriers,
                                                           std::vector<VkImageMemoryBarrier2> &imageByRegionBarriers,
                                                           std::vector<VkBufferMemoryBarrier2> &bufferBarriers) {
        RaytracingStageBase::RecordFrameBarriers(cmdBuffer, renderInfo, imageFullBarriers, imageByRegionBarriers, bufferBarriers);
        imageFullBarriers.push_back(renderInfo.GetImageLayoutCache().MakeBarrier(mIrradianceCache.GetTempImage(), core::ImageLayoutCache::Barrier2{
                .SrcStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .SrcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .DstStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .DstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
                .NewLayout     = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL
        }));
        imageFullBarriers.push_back(renderInfo.GetImageLayoutCache().MakeBarrier(mIrradianceCache.GetIndirectImage(), core::ImageLayoutCache::Barrier2{
                .SrcStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .SrcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT,
                .DstStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .DstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                .NewLayout     = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL
        }));
    }

    void IrradianceCacheIndirectStage::RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        RaytracingStageBase::RecordFrameBind(cmdBuffer, renderInfo);
        mIrradianceCacheShaderAccess->pushConstants(cmdBuffer, renderInfo);
    }

    void IrradianceCacheIndirectStage::RecordFrameTraceRays(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry = mPipeline.GetRaygenSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry = mPipeline.GetMissSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry = mPipeline.GetHitSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry = mPipeline.GetCallablesSbt().GetAddressRegion();
        VkExtent3D imageExtend = mIrradianceCache.GetImageExtent();
        mContext->VkbDispatchTable->cmdTraceRaysKHR(cmdBuffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry,
                                                    imageExtend.width, imageExtend.height, imageExtend.depth);
    }
}
