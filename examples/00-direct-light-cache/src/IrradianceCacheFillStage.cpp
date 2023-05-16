#include "IrradianceCacheFillStage.hpp"
#include "core/foray_samplercollection.hpp"
#include "scene/components/foray_meshinstance.hpp"
#include "scene/foray_scene.hpp"
#include "scene/globalcomponents/foray_cameramanager.hpp"
#include "scene/globalcomponents/foray_geometrymanager.hpp"
#include "scene/globalcomponents/foray_materialmanager.hpp"
#include "scene/globalcomponents/foray_texturemanager.hpp"
#include "scene/globalcomponents/foray_tlasmanager.hpp"
#include "util/foray_shaderstagecreateinfos.hpp"

namespace foray::irradiance_cache {

    // bindpoints
    const uint32_t BINDPOINT_LIGHTS = 11;

    // shaders
    const std::string FOLDER_IRRADIANCE_CACHE = "shaders/irradiancecachefill/";
    const std::string RAYGEN_FILE = FOLDER_IRRADIANCE_CACHE + "raygen.rgen";

    IrradianceCacheFillShaders::IrradianceCacheFillShaders(IrradianceCacheFillStage &s) : mStage(s) {
        foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR}};
        s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));

        s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
        mVisiTest.Init(s.mContext, s.mShaderKeys, s.mPipeline, 0);

        s.mPipeline.Build(s.mContext, s.mPipelineLayout);
    }

    IrradianceCacheFillShaders::~IrradianceCacheFillShaders() {
        mStage.mPipeline.Destroy();
    }

    void IrradianceCacheFillStage::ApiCreateRtPipeline() {
        mShaders.emplace(*this);
    }

    void IrradianceCacheFillStage::ApiDestroyRtPipeline() {
        mShaders.reset();
    }

    // stage
    IrradianceCacheFillStage::IrradianceCacheFillStage(IrradianceCache &irradianceCache, scene::Scene *scene, core::CombinedImageSampler *envMap,
                                                       core::ManagedImage *noiseImage) :
            mIrradianceCache(irradianceCache) {
        // disable push constant
        mRngSeedPushCOffset = ~0;

        mLightManager = scene->GetComponent<foray::scene::gcomp::LightManager>();
        Init(irradianceCache.GetContext(), scene, envMap, noiseImage);
    }

    void IrradianceCacheFillStage::CreateOutputImages() {
        // IrradianceCache is managed separately from the FillStage
        // also prevents it from being recreated when swapchain size changes
    }

    void IrradianceCacheFillStage::CreateOrUpdateDescriptors() {
        using namespace stages::rtbindpoints;

        mDescriptorSet.SetDescriptorAt(BINDPOINT_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(),
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, foray::stages::RTSTAGEFLAGS);
        mDescriptorSet.SetDescriptorAt(BIND_OUT_IMAGE, mIrradianceCache.GetImage(), VK_IMAGE_LAYOUT_GENERAL, nullptr,
                                       VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,stages::RTSTAGEFLAGS);
        RaytracingStageBase::CreateOrUpdateDescriptors();
    }

    void IrradianceCacheFillStage::CreatePipelineLayout() {
        mIrradianceCacheShaderAccess.emplace(mIrradianceCache, mPipelineLayout, stages::RTSTAGEFLAGS);
        RaytracingStageBase::CreatePipelineLayout();
    }

    void IrradianceCacheFillStage::RecordFrameBarriers(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo, std::vector<VkImageMemoryBarrier2> &imageBarriers,
                                                       std::vector<VkBufferMemoryBarrier2> &bufferBarriers) {
        RaytracingStageBase::RecordFrameBarriers(cmdBuffer, renderInfo, imageBarriers, bufferBarriers);

        imageBarriers.push_back(renderInfo.GetImageLayoutCache().MakeBarrier(mIrradianceCache.GetImage(), core::ImageLayoutCache::Barrier2{
                .SrcStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                .SrcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
                .DstStageMask  = VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR,
                .DstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
                .NewLayout     = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL
        }));
    }

    void IrradianceCacheFillStage::RecordFrameBind(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        RaytracingStageBase::RecordFrameBind(cmdBuffer, renderInfo);
        mIrradianceCacheShaderAccess->pushConstants(cmdBuffer, renderInfo);
    }

    void IrradianceCacheFillStage::RecordFrameTraceRays(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        VkStridedDeviceAddressRegionKHR raygen_shader_sbt_entry = mPipeline.GetRaygenSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR miss_shader_sbt_entry = mPipeline.GetMissSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR hit_shader_sbt_entry = mPipeline.GetHitSbt().GetAddressRegion();
        VkStridedDeviceAddressRegionKHR callable_shader_sbt_entry = mPipeline.GetCallablesSbt().GetAddressRegion();
        VkExtent3D imageExtend = mIrradianceCache.GetImageExtent();
        mContext->VkbDispatchTable->cmdTraceRaysKHR(cmdBuffer, &raygen_shader_sbt_entry, &miss_shader_sbt_entry, &hit_shader_sbt_entry, &callable_shader_sbt_entry,
                                                    imageExtend.width, imageExtend.height, imageExtend.depth);
    }
}
