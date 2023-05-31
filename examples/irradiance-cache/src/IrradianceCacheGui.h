#pragma once

namespace foray::irradiance_cache {

    class IrradianceCacheApp;

    class IrradianceCacheGui {
    public:
        explicit IrradianceCacheGui(IrradianceCacheApp &app);
        void ImGui();

    private:
        IrradianceCacheApp &mApp;
        float accumQuality = 1;
    };

};

