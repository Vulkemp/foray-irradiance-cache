#pragma once

#include "stages/foray_defaultraytracingstage.hpp"
#include "scene/globalcomponents/foray_lightmanager.hpp"
#include "VisiTest.h"

namespace foray::irradiance_cache {

    inline const std::string FOLDER_FINALRT = "mShaders/finalrt/";
    inline const std::string RAYGEN_FILE = FOLDER_FINALRT + "raygen.rgen";
    inline const std::string CLOSESTHIT_FILE = FOLDER_FINALRT + "closesthit.rchit";
    inline const std::string ANYHIT_FILE = FOLDER_FINALRT + "anyhit.rahit";
    inline const std::string MISS_FILE = FOLDER_FINALRT + "miss.rmiss";

    const uint32_t BINDPOINT_LIGHTS = 11;

    class FinalRTStage;

    class FinalRTShaders {
    public:
        explicit FinalRTShaders(FinalRTStage *s);
        ~FinalRTShaders();

    private:
        FinalRTStage *mFinalRtStage;
        core::ShaderModule mRaygen;
        core::ShaderModule mClosestHit;
        core::ShaderModule mAnyHit;
        core::ShaderModule mMiss;
        VisiTest mVisiTest;
    };

    class FinalRTStage : public stages::DefaultRaytracingStageBase {
        friend FinalRTShaders;

    public:
        virtual void Init(core::Context *context, scene::Scene *scene);

    protected:
        void ApiCreateRtPipeline() override;

        void ApiDestroyRtPipeline() override;

        void CreateOrUpdateDescriptors() override;

    private:
        std::optional<FinalRTShaders> mShaders;
        scene::gcomp::LightManager *mLightManager;
    };

}

