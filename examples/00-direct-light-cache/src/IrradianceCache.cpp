#include "IrradianceCache.h"

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
        mImage.Create(context, usage, VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT, imageExtend, "Irradiance Cache");
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

    IrradianceCacheShaderAccess::IrradianceCacheShaderAccess(IrradianceCache &irradianceCache, util::PipelineLayout &pipelineLayout, VkShaderStageFlags stageFlags) :
            irradianceCache(irradianceCache),
            pipelineLayout(pipelineLayout),
            stageFlags(stageFlags) {
        pipelineLayout.AddPushConstantRange<PushConstant>(stageFlags, 0);
    }

    void IrradianceCacheShaderAccess::pushConstants(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo) {
        auto imageExtent = irradianceCache.GetImageExtent();
        PushConstant ps = {
                glm::vec4(irradianceCache.mOrigin, 0.f),
                glm::vec4(irradianceCache.mExtent, 0.f),
                glm::vec4(imageExtent.width, imageExtent.height, imageExtent.depth, 0.f),
                (uint32_t) renderInfo.GetFrameNumber()
        };
        vkCmdPushConstants(cmdBuffer, pipelineLayout, stageFlags, 0, sizeof(ps), &ps);
    }
}
