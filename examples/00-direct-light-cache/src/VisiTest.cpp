#include "VisiTest.h"
#include "core/foray_shadermanager.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace foray::irradiance_cache {

    const std::string FOLDER_VISITEST = "shaders/visitest/";
    const std::string VISI_MISS_FILE = FOLDER_VISITEST + "miss.rmiss";
    const std::string VISI_ANYHIT_FILE = FOLDER_VISITEST + "anyhit.rahit";

    void VisiTest::VisiTest::Init(core::Context *context, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline, rtpipe::GroupIndex groupId) {
        core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR, EXAMPLE_SHADER_DIR}};

        shaderKeys.push_back(mVisiMiss.CompileFromSource(context, VISI_MISS_FILE, options));
        shaderKeys.push_back(mVisiAnyHit.CompileFromSource(context, VISI_ANYHIT_FILE, options));

        pipeline.GetHitSbt().SetGroup(groupId, nullptr, &mVisiAnyHit, nullptr);
        pipeline.GetMissSbt().SetGroup(groupId, &mVisiMiss);
    }

}
