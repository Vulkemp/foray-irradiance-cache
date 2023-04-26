#pragma once

#include "stages/foray_renderstage.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace foray::irradiance_cache {

    struct VisiTest {
    public:
        VisiTest() = default;
        void Init(core::Context *context, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline, rtpipe::GroupIndex groupId);

        core::ShaderModule mVisiMiss;
        core::ShaderModule mVisiAnyHit;
    };

}
