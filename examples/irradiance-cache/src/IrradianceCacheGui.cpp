#include "IrradianceCacheGui.h"
#include "IrradianceCacheApp.h"
#include "scene/globalcomponents/foray_cameramanager.hpp"
#include "scene/components/foray_camera.hpp"
#include "scene/components/foray_transform.hpp"
#include <imgui/imgui.h>

namespace foray::irradiance_cache {

    IrradianceCacheGui::IrradianceCacheGui(IrradianceCacheApp &app)
            : mApp(app) {}

    void IrradianceCacheGui::ImGui() {
        ImGui::Begin("window");

        foray::base::RenderLoop::FrameTimeAnalysis analysis = mApp.GetRenderLoop().AnalyseFrameTimes();
        ImGui::Text("FPS: %f avg %f min", analysis.Count > 0 ? 1.f / analysis.AvgFrameTime : 0, analysis.Count > 0 ? 1.f / analysis.MaxFrameTime : 0);

        glm::vec3 cameraPos = mApp.GetScene()->GetComponent<foray::scene::gcomp::CameraManager>()->GetSelectedCamera()->GetNode()->GetTransform()->GetTranslation();
        ImGui::Text("Camera: %f %f %f", cameraPos.x, cameraPos.y, cameraPos.z);

        auto &irradianceCache = *mApp.GetIrradianceCache();
        std::string modeName(NAMEOF_ENUM(irradianceCache.GetMode()));
        if (ImGui::BeginCombo("Mode", modeName.c_str())) {
            for (uint32_t i = 0; i < (uint32_t) IrradianceCacheMode::MAX_ENUM; i++) {
                auto mode = (IrradianceCacheMode) i;
                modeName = std::string(NAMEOF_ENUM(mode));
                if (ImGui::MenuItem(modeName.c_str())) {
                    irradianceCache.SetMode(mode);
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("Clear Irradiance Cache")) {
            irradianceCache.clearCache();
        }

        int tracesPerFrame = (int) irradianceCache.GetTracesPerFrame();
        ImGui::SliderInt("Traces per frame", &tracesPerFrame, 1, 256, "%u", ImGuiSliderFlags_Logarithmic);
        irradianceCache.SetTracesPerFrame(tracesPerFrame > 1 ? (uint32_t) tracesPerFrame : 1);

        float optimalAccumFactor = IrradianceCache::optimalAccumulationRateForTraces(tracesPerFrame);
        ImGui::SliderFloat("Accumulation Quality", &accumQuality, 1.f / 64.f, 4, "%.4f", ImGuiSliderFlags_Logarithmic);
        float accumFactor = std::clamp(optimalAccumFactor / accumQuality, 0.f, 1.f);
        ImGui::SliderFloat("Accumulation Factor", &accumFactor, 0, 0.5, "%.4f");
        accumQuality = optimalAccumFactor / accumFactor;
        irradianceCache.SetAccumulationFactor(accumFactor);

        ImGui::SliderFloat("Normal Offset Factor", &irradianceCache.GetNormalOffsetFactor(), 0, 4, "%.2f");

        ImGui::Checkbox("Allow skip IC", &mApp.GetAllowSkipIC());

        ImGui::End();
    }

}
