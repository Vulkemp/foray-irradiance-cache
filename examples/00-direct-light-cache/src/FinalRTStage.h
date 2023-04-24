#pragma once

#include "stages/foray_defaultraytracingstage.hpp"
#include "scene/globalcomponents/foray_lightmanager.hpp"

namespace irradiance_cache {

    inline const std::string SHADER_FOLDER = "shaders/finalrt/";
    inline const std::string RAYGEN_FILE = SHADER_FOLDER + "raygen.rgen";
    inline const std::string CLOSESTHIT_FILE = SHADER_FOLDER + "closesthit.rchit";
    inline const std::string ANYHIT_FILE = SHADER_FOLDER + "anyhit.rahit";
    inline const std::string MISS_FILE = SHADER_FOLDER + "miss.rmiss";
    inline const std::string VISI_MISS_FILE = SHADER_FOLDER + "visibilitytest/miss.rmiss";
    inline const std::string VISI_ANYHIT_FILE = SHADER_FOLDER + "visibilitytest/anyhit.rahit";

    const uint32_t BINDPOINT_LIGHTS = 11;

    class FinalRTStage;

    class FinalRTShaders {
        FinalRTStage *finalRtStage;
        foray::core::ShaderModule mRaygen;
        foray::core::ShaderModule mClosestHit;
        foray::core::ShaderModule mAnyHit;
        foray::core::ShaderModule mMiss;
        foray::core::ShaderModule mVisiMiss;
        foray::core::ShaderModule mVisiAnyHit;

    public:
        explicit FinalRTShaders(FinalRTStage *s);

//        FinalRTShaders(FinalRTShaders&& other) = default;

        ~FinalRTShaders();
    };

    class FinalRTStage : public foray::stages::DefaultRaytracingStageBase {
        friend FinalRTShaders;

    public:
        virtual void Init(foray::core::Context *context, foray::scene::Scene *scene);

    protected:
        void ApiCreateRtPipeline() override;

        void ApiDestroyRtPipeline() override;

        void CreateOrUpdateDescriptors() override;

    private:
        std::optional<FinalRTShaders> shaders;
        foray::scene::gcomp::LightManager *mLightManager;
    };

}

