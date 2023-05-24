#pragma once

#include "stages/foray_renderstage.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace foray::irradiance_cache {

    struct VisiTest {
    public:
        VisiTest(core::Context *context, core::ShaderCompilerConfig &options, int groupId);
        void Compile(core::Context *context, core::ShaderCompilerConfig &options, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline);

        rtpipe::GroupIndex mGroupId;
        core::ShaderModule mVisiMiss;
        core::ShaderModule mVisiAnyHit;
    };

}
