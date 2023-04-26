#include "FinalRTStage.h"
#include "scene/foray_scene.hpp"

namespace foray::irradiance_cache {

    FinalRTShaders::FinalRTShaders(FinalRTStage *s2) {
        FinalRTStage &s = *s2;
        this->mFinalRtStage = &s;

        foray::core::ShaderCompilerConfig options{.IncludeDirs = {FORAY_SHADER_DIR}};

        s.mShaderKeys.push_back(mRaygen.CompileFromSource(s.mContext, RAYGEN_FILE, options));
        s.mShaderKeys.push_back(mClosestHit.CompileFromSource(s.mContext, CLOSESTHIT_FILE, options));
        s.mShaderKeys.push_back(mAnyHit.CompileFromSource(s.mContext, ANYHIT_FILE, options));
        s.mShaderKeys.push_back(mMiss.CompileFromSource(s.mContext, MISS_FILE, options));

        s.mPipeline.GetRaygenSbt().SetGroup(0, &mRaygen);
        s.mPipeline.GetHitSbt().SetGroup(0, &mClosestHit, &mAnyHit, nullptr);
        s.mPipeline.GetMissSbt().SetGroup(0, &mMiss);
        mVisiTest.Init(s.mContext, s.mShaderKeys, s.mPipeline, 1);

        s.mPipeline.Build(s.mContext, s.mPipelineLayout);
    }

    FinalRTShaders::~FinalRTShaders() {
        this->mFinalRtStage->mPipeline.Destroy();
    }

    void FinalRTStage::ApiCreateRtPipeline() {
        mShaders.emplace(this);
    }

    void FinalRTStage::ApiDestroyRtPipeline() {
        mShaders.reset();
    }

    void FinalRTStage::CreateOrUpdateDescriptors() {
        mDescriptorSet.SetDescriptorAt(BINDPOINT_LIGHTS, mLightManager->GetBuffer().GetVkDescriptorInfo(), VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                       foray::stages::RTSTAGEFLAGS);
        DefaultRaytracingStageBase::CreateOrUpdateDescriptors();
    }

    void FinalRTStage::Init(foray::core::Context *context, foray::scene::Scene *scene) {
        mLightManager = scene->GetComponent<foray::scene::gcomp::LightManager>();
        foray::stages::DefaultRaytracingStageBase::Init(context, scene);
    }

}
