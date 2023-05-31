#include "ProbeMat.h"
#include "core/foray_shadermanager.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

namespace foray::irradiance_cache {

    const std::string FOLDER_ProbeMat = "shaders/probemat/";
    const std::string PROBEMAT_MISS_FILE = FOLDER_ProbeMat + "miss.rmiss";
    const std::string PROBEMAT_CLOSESTHIT_FILE = FOLDER_ProbeMat + "closesthit.rchit";
    const std::string PROBEMAT_ANYHIT_FILE = FOLDER_ProbeMat + "anyhit.rahit";

    ProbeMat::ProbeMat(core::Context *context, core::ShaderCompilerConfig &options, rtpipe::GroupIndex groupId): mGroupId(groupId) {
        options.Definitions.push_back(std::string("PROBEMAT_GROUP_ID=").append(std::to_string(groupId)));
    }

    void ProbeMat::Compile(core::Context *context, core::ShaderCompilerConfig &options, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline) {
        shaderKeys.push_back(mProbeMatMiss.CompileFromSource(context, PROBEMAT_MISS_FILE, options));
        shaderKeys.push_back(mProbeMatClosestHit.CompileFromSource(context, PROBEMAT_CLOSESTHIT_FILE, options));
        shaderKeys.push_back(mProbeMatAnyHit.CompileFromSource(context, PROBEMAT_ANYHIT_FILE, options));

        pipeline.GetHitSbt().SetGroup(mGroupId, &mProbeMatClosestHit, &mProbeMatAnyHit, nullptr);
        pipeline.GetMissSbt().SetGroup(mGroupId, &mProbeMatMiss);
    }
}
