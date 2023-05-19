#pragma once

namespace foray::irradiance_cache {

    enum class IrradianceCacheMode {
        INDIRECT_ILLUMINATION,
        DIRECT_ILLUMINATION,
        DEBUG_PATTERN,
        MAX_ENUM,
        DEFAULT = DIRECT_ILLUMINATION,
    };

}
