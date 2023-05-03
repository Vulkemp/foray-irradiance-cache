#pragma once

#include "foray_glm.hpp"
#include "core/foray_managed3dimage.hpp"
#include "base/foray_framerenderinfo.hpp"
#include "util/foray_pipelinelayout.hpp"

namespace foray::irradiance_cache {

    struct PushConstant {
        glm::vec4 origin;
        glm::vec4 extent;
        glm::vec4 imageExtent;
        uint32_t RngSeed;
    };

    class IrradianceCacheShaderAccess;

    class IrradianceCache {
        friend IrradianceCacheShaderAccess;
    public:
        IrradianceCache(foray::core::Context *context, glm::vec3 origin, glm::vec3 extent, VkExtent3D imageExtend);

        // @brief select an imageExtent so that probes are PROBE_DISTANCE always apart, may increase extent to match PROBE_DISTANCE
        static VkExtent3D calculateImageExtend(glm::vec3 &extent, glm::vec3 probeDistance);

        FORAY_GETTER_V(Origin);

        FORAY_GETTER_V(Extent);

        FORAY_GETTER_V(Context);

        FORAY_GETTER_R(Image);

        VkExtent3D GetImageExtent() {
            return mImage.GetExtent3D();
        }

    private:
        foray::core::Context *mContext;
        glm::vec3 mOrigin;
        glm::vec3 mExtent;

        foray::core::Managed3dImage mImage;
    };

    class IrradianceCacheShaderAccess {
        friend IrradianceCache;
    public:
        IrradianceCacheShaderAccess(IrradianceCache &irradianceCache, util::PipelineLayout &pipelineLayout, VkShaderStageFlags stageFlags);

        void pushConstants(VkCommandBuffer cmdBuffer, base::FrameRenderInfo &renderInfo);

    private:
        IrradianceCache &irradianceCache;
        util::PipelineLayout &pipelineLayout;
        VkShaderStageFlags stageFlags;
    };

}
