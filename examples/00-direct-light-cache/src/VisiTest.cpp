#include "VisiTest.h"
#include "core/foray_shadermanager.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace foray::irradiance_cache {

    const std::string FOLDER_VISITEST = "shaders/visitest/";
    const std::string VISI_MISS_FILE = FOLDER_VISITEST + "miss.rmiss";
    const std::string VISI_ANYHIT_FILE = FOLDER_VISITEST + "anyhit.rahit";

    VisiTest::VisiTest(core::Context *context, core::ShaderCompilerConfig &options, rtpipe::GroupIndex groupId): mGroupId(groupId) {
        options.Definitions.push_back(std::string("VISITEST_GROUP_ID=").append(std::to_string(groupId)));
    }

    void VisiTest::Compile(core::Context *context, core::ShaderCompilerConfig &options, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline) {
        shaderKeys.push_back(mVisiMiss.CompileFromSource(context, VISI_MISS_FILE, options));
        shaderKeys.push_back(mVisiAnyHit.CompileFromSource(context, VISI_ANYHIT_FILE, options));

        pipeline.GetHitSbt().SetGroup(mGroupId, nullptr, &mVisiAnyHit, nullptr);
        pipeline.GetMissSbt().SetGroup(mGroupId, &mVisiMiss);
    }
}
