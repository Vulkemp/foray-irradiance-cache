#pragma once

#include "foray_glm.hpp"
#include "core/foray_managed3dimage.hpp"
#include "base/foray_framerenderinfo.hpp"
#include "util/foray_pipelinelayout.hpp"
#include "IrrradianceCacheMode.h"
#include "core/foray_samplercollection.hpp"

namespace foray::irradiance_cache {

    struct PushConstant {
        // w = IrradianceCacheMode
        glm::vec4 origin;
        // w unused
        glm::vec4 extent;
        // w unused
        glm::vec4 imageExtent;
        // x IrradianceCacheMode
        // y bool clearCache
        // zw unused
        glm::uvec4 config;
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

        FORAY_GETTER_V(Mode);

        FORAY_GETTER_R(IndirectImage);

        FORAY_GETTER_R(TempImage);

        inline VkExtent3D GetImageExtent() {
            return mIndirectImage.GetExtent3D();
        }

        inline void SetMode(IrradianceCacheMode mode) {
            mMode = mode;
            clearCache();
        }

        inline void clearCache() {
            mClearCache = true;
        }

    private:
        foray::core::Context *mContext;
        glm::vec3 mOrigin;
        glm::vec3 mExtent;
        IrradianceCacheMode mMode = IrradianceCacheMode::DEFAULT;
        bool mClearCache = false;

        foray::core::Managed3dImage mIndirectImage;
        foray::core::Managed3dImage mTempImage;
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
