#include "IrradianceCache.h"
#include "core/foray_samplercollection.hpp"

namespace foray::irradiance_cache {

    template<glm::length_t L, typename T, glm::qualifier Q>
    bool isPositive(glm::vec<L, T, Q> v) {
        for (glm::length_t i = 0; i < L; i++)
            if (v[i] < 0)
                return false;
        return true;
    }

    IrradianceCache::IrradianceCache(foray::core::Context *context, glm::vec3 origin, glm::vec3 extent, VkExtent3D imageExtend) :
            mContext(context),
            mOrigin(origin),
            mExtent(extent) {
        assert(isPositive(extent));
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        mIndirectImage.Create(context, usage, VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT, imageExtend, "Irradiance Cache Indirect");
        mTempImage.Create(context, usage, VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT, imageExtend, "Irradiance Cache Temp");
    }

    VkExtent3D IrradianceCache::calculateImageExtend(glm::vec3 &extent, glm::vec3 probeDistance) {
        assert(isPositive(extent));
        assert(isPositive(probeDistance));
        glm::vec3 probesF = extent / probeDistance;
        glm::uvec3 probes(probesF);
        for (glm::length_t i = 0; i < 3; i++) {
            if ((float) probes[i] < probesF[i])
                probes[i]++;
        }
        extent = glm::vec3(probes) * probeDistance;
        return VkExtent3D{probes.x, probes.y, probes.z};
    }

    float IrradianceCache::optimalAccumulationRateForTraces(uint32_t tracesPerFrame) {
        return (float) tracesPerFrame / 64.f / 100.f;
    }

    void IrradianceCache::frameFinished() {
        // frame is done, stop clearing cache
        // requires a queue so clearing cache from ImGui doesn't get cleared immediately
        mClearCache = mClearCacheQueued;
        mClearCacheQueued = false;
    }

    IrradianceCacheShaderAccess::IrradianceCacheShaderAccess(IrradianceCache &irradianceCache, util::PipelineLayout &pipelineLayout, VkShaderStageFlags stageFlags) :
            irradianceCache(irradianceCache),
            pipelineLayout(pipelineLayout),
            stageFlags(stageFlags) {
        pipelineLayout.AddPushConstantRange<PushConstant>(stageFlags, 0);
    }

    void IrradianceCacheShaderAccess::pushConstants(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        auto imageExtent = irradianceCache.GetImageExtent();
        float ONE = 1;
        PushConstant ps = {
                glm::vec4(irradianceCache.mOrigin, irradianceCache.mNormalOffsetFactor),
                glm::vec4(irradianceCache.mExtent, 0.f),
                glm::vec4(imageExtent.width, imageExtent.height, imageExtent.depth, 0.f),
                glm::uvec4(
                        (uint32_t) irradianceCache.mMode,
                        irradianceCache.mClearCache ? 1 : 0,
                        irradianceCache.mTracesPerFrame,
                        *reinterpret_cast<uint32_t*>(irradianceCache.mClearCache ? &ONE : &irradianceCache.mAccumulationFactor)
                ),
                (uint32_t) renderInfo.GetFrameNumber()
        };
        vkCmdPushConstants(cmdBuffer, pipelineLayout, stageFlags, 0, sizeof(ps), &ps);
    }
}
