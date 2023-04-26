#include "FinalRTStage.h"
#include "scene/foray_scene.hpp"

irradiance_cache::FinalRTShaders::FinalRTShaders(irradiance_cache::FinalRTStage *s2) {
    irradiance_cache::FinalRTStage &s = *s2;
    this->finalRtStage = &s;

    foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR}};

    s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));
    s.mShaderKeys.push_back(mClosestHit.CompileFromSource(s.mContext, CLOSESTHIT_FILE, options));
    s.mShaderKeys.push_back(mAnyHit.CompileFromSource(s.mContext, ANYHIT_FILE, options));
    s.mShaderKeys.push_back(mMiss.CompileFromSource(s.mContext, MISS_FILE, options));
    s.mShaderKeys.push_back(mVisiMiss.CompileFromSource(s.mContext, VISI_MISS_FILE, options));
    s.mShaderKeys.push_back(mVisiAnyHit.CompileFromSource(s.mContext, VISI_ANYHIT_FILE, options));

    s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
    s.mPipeline.GetHitSbt().SetGroup(0, &mClosestHit, &mAnyHit, nullptr);
    s.mPipeline.GetHitSbt().SetGroup(1, nullptr, &mVisiAnyHit, nullptr);
    s.mPipeline.GetMissSbt().SetGroup(0, &mMiss);
    s.mPipeline.GetMissSbt().SetGroup(1, &mVisiMiss);
    s.mPipeline.Build(s.mContext, s.mPipelineLayout);
}

irradiance_cache::FinalRTShaders::~FinalRTShaders() {
    this->finalRtStage->mPipeline.Destroy();
}

void irradiance_cache::FinalRTStage::ApiCreateRtPipeline() {
    shaders.emplace(this);
}

void irradiance_cache::FinalRTStage::ApiDestroyRtPipeline() {
    shaders.reset();
}

void irradiance_cache::FinalRTStage::CreateOrUpdateDescriptors() {
    mDescriptorSet.SetDescriptorAt(BINDPOINT_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                   foray::stages::RTSTAGEFLAGS);
    DefaultRaytracingStageBase::CreateOrUpdateDescriptors();
}

void irradiance_cache::FinalRTStage::Init(foray::core::Context *context, foray::scene::Scene *scene) {
    mLightManager = scene->GetComponent<foray::scene::gcomp::LightManager>();
    foray::stages::DefaultRaytracingStageBase::Init(context, scene);
}
