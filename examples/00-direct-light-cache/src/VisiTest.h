#pragma once

#include "stages/foray_renderstage.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace irradiance_cache {

    struct VisiTest {
    public:
        VisiTest() = default;
        void Init(foray::core::Context *context, std::vector<uint64_t> &shaderKeys, foray::rtpipe::RtPipeline &pipeline, foray::rtpipe::GroupIndex groupId);

        foray::core::ShaderModule mVisiMiss;
        foray::core::ShaderModule mVisiAnyHit;
    };

}
