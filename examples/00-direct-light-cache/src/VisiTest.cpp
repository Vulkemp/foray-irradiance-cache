#include "VisiTest.h"
#include "core/foray_shadermanager.hpp"
#include "rtpipe/foray_rtpipeline.hpp"

using namespace foray;

inline const std::string FOLDER_VISITEST = "shaders/visitest/";
inline const std::string VISI_MISS_FILE = FOLDER_VISITEST + "miss.rmiss";
inline const std::string VISI_ANYHIT_FILE = FOLDER_VISITEST + "anyhit.rahit";

void irradiance_cache::VisiTest::VisiTest::Init(core::Context* context, std::vector<uint64_t> &shaderKeys, rtpipe::RtPipeline &pipeline, rtpipe::GroupIndex groupId) {
    foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR}};

    shaderKeys.push_back(mVisiMiss.CompileFromSource(context, VISI_MISS_FILE, options));
    shaderKeys.push_back(mVisiAnyHit.CompileFromSource(context, VISI_ANYHIT_FILE, options));

    pipeline.GetHitSbt().SetGroup(groupId, nullptr, &mVisiAnyHit, nullptr);
    pipeline.GetMissSbt().SetGroup(groupId, &mVisiMiss);
}
